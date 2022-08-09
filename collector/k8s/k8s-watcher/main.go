// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"log"
	"sort"
	"strconv"
	"strings"
	"time"

	"ebpf.net/collector"

	"google.golang.org/grpc"

	appsv1 "k8s.io/api/apps/v1"
	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/watch"
	"k8s.io/client-go/kubernetes"
	"k8s.io/client-go/rest"
)

var (
	server_address = flag.String("server-address", "localhost:8712",
		"Where traffic is sent to. It should be in hostname:port format.")
	local_test = flag.Bool("local-test", false,
		"If true, do not connect to K8S; instead, generate synthetic traffic for test.")
	log_to_stderr = flag.Bool("log-to-stderr", false,
		"If true, perform additional logging to stderr.")
)

func logmsg(msg string) {
	if *log_to_stderr {
		log.Println(msg)
	}
}

func info_event_type(watch_event_type watch.EventType) (collector.Info_Event, error) {
	switch watch_event_type {
	case watch.Added:
		return collector.Info_ADDED, nil
	case watch.Modified:
		return collector.Info_MODIFIED, nil
	case watch.Deleted:
		return collector.Info_DELETED, nil
	case watch.Error:
		return collector.Info_ERROR, nil
	}

	return collector.Info_ERROR, errors.New("unexpected watch event")
}

func extract_owner(objm metav1.ObjectMeta) *collector.OwnerInfo {
	for _, owner_ref := range objm.OwnerReferences {
		if owner_ref.Controller != nil && *owner_ref.Controller {
			owner_info := &collector.OwnerInfo{
				Uid:  string(owner_ref.UID),
				Name: owner_ref.Name,
				Kind: owner_ref.Kind,
			}

			return owner_info
		}
	}

	return nil
}

func extract_version(pod *corev1.Pod) string {
	var tags []string
	for _, cs := range pod.Status.ContainerStatuses {
		tags = append(tags, fmt.Sprintf("'%s'", cs.Image))
	}
	sort.Strings(tags)
	return strings.Join(tags, ",")
}

func extract_containers(pod *corev1.Pod) []*collector.ContainerInfo {
	containers := make([]*collector.ContainerInfo, 0, len(pod.Status.ContainerStatuses))
	for _, cs := range pod.Status.ContainerStatuses {
		container := &collector.ContainerInfo{}
		container.Id = cs.ContainerID
		container.Name = cs.Name
		container.Image = cs.Image
		containers = append(containers, container)
	}
	return containers
}

func make_pod_info(pod *corev1.Pod) *collector.PodInfo {
	return &collector.PodInfo{
		Uid:            string(pod.ObjectMeta.UID),
		Ip:             pod.Status.PodIP,
		Name:           pod.ObjectMeta.Name,
		Owner:          extract_owner(pod.ObjectMeta),
		Ns:             pod.ObjectMeta.Namespace,
		Version:        extract_version(pod),
		IsHostNetwork:  pod.Spec.HostNetwork,
		ContainerInfos: extract_containers(pod),
	}
}

func make_rs_info(rs *appsv1.ReplicaSet) *collector.ReplicaSetInfo {
	return &collector.ReplicaSetInfo{
		Uid:   string(rs.ObjectMeta.UID),
		Owner: extract_owner(rs.ObjectMeta),
	}
}

func make_collector_info_from_pod(event_type collector.Info_Event, pod_info *collector.PodInfo) *collector.Info {
	return &collector.Info{
		Type:    collector.Info_K8S_POD,
		Event:   event_type,
		PodInfo: pod_info,
	}
}

func make_collector_info_from_replicaset(event_type collector.Info_Event, rs_info *collector.ReplicaSetInfo) *collector.Info {
	return &collector.Info{
		Type:   collector.Info_K8S_REPLICASET,
		Event:  event_type,
		RsInfo: rs_info,
	}
}

func send_info(info *collector.Info, stream collector.Collector_CollectClient) error {
	logmsg("Sending: " + info.String())
	return stream.Send(info)
}

// Handles Pod event from watcher.
//
// Returns the version of the Pod object, and the error object, if any.
func handle_pod_event(event watch.Event, stream collector.Collector_CollectClient) (*string, error) {
	event_type, err := info_event_type(event.Type)
	if err != nil {
		return nil, err
	}

	pod, ok := event.Object.(*corev1.Pod)
	if !ok || pod == nil {
		return nil, errors.New("errorenous Pod watch event")
	}

	err = send_info(make_collector_info_from_pod(event_type, make_pod_info(pod)), stream)
	return &pod.ObjectMeta.ResourceVersion, err
}

// Handles ReplicaSet event from watcher.
//
// Returns the version of the Pod object, and the error object, if any.
func handle_rs_event(event watch.Event, stream collector.Collector_CollectClient) (*string, error) {
	event_type, err := info_event_type(event.Type)
	if err != nil {
		return nil, err
	}

	rs, ok := event.Object.(*appsv1.ReplicaSet)
	if !ok || rs == nil {
		return nil, errors.New("errorenous ReplicaSet watch event")
	}

	err = send_info(make_collector_info_from_replicaset(event_type, make_rs_info(rs)), stream)
	return &rs.ObjectMeta.ResourceVersion, err
}

// For test the connection w/o connecting to k8s master.
func run_local_test(stream collector.Collector_CollectClient) error {
	for i := 0; ; i++ {
		s := strconv.Itoa(i)
		rs_info := &collector.ReplicaSetInfo{
			Uid: "RS-UID-" + s,
			Owner: &collector.OwnerInfo{
				Uid:  "RS-OWNER-UID-" + s,
				Name: "RS-OWNER-Name-" + s,
				Kind: "Deployment",
			},
		}

		info := &collector.Info{
			Type:   collector.Info_K8S_REPLICASET,
			Event:  collector.Info_ADDED,
			RsInfo: rs_info,
		}

		logmsg("Sending: " + info.String())
		err := stream.Send(info)
		if err != nil {
			return err
		}

		pod_info := &collector.PodInfo{
			Uid:  "POD-UID-" + s,
			Ip:   "192.168.1.1",
			Ns:   "POD-NS-" + s,
			Name: "POD-N-" + s,
			Owner: &collector.OwnerInfo{
				Uid:  "RS-UID-" + s,
				Name: "RSS-OWNER-Name-" + s,
				Kind: "ReplicaSet",
			},
		}

		info2 := &collector.Info{
			Type:    collector.Info_K8S_POD,
			Event:   collector.Info_ADDED,
			PodInfo: pod_info,
		}

		logmsg("Sending: " + info2.String())
		err = stream.Send(info2)
		if err != nil {
			return err
		}

		info3 := &collector.Info{
			Type:    collector.Info_K8S_POD,
			Event:   collector.Info_DELETED,
			PodInfo: pod_info,
		}
		time.Sleep(100 * time.Millisecond)

		logmsg("Sending: " + info3.String())
		err = stream.Send(info3)
		if err != nil {
			return err
		}

		pod_info_no_owner := &collector.PodInfo{
			Uid:  "POD-UID-NO-OWNER" + s,
			Ip:   "192.168.1.1",
			Name: "POD-NO-NAME-" + s,
			Ns:   "POD-NS-NO-" + s,
		}

		info4 := &collector.Info{
			Type:    collector.Info_K8S_POD,
			Event:   collector.Info_ADDED,
			PodInfo: pod_info_no_owner,
		}

		time.Sleep(100 * time.Millisecond)
		logmsg("Sending: " + info4.String())
		err = stream.Send(info4)
		if err != nil {
			return err
		}

		time.Sleep(500 * time.Millisecond)
	}
}

func run() error {
	logmsg("Connect to Collector service.")

	conn, err := grpc.Dial(*server_address, grpc.WithInsecure())
	if err != nil {
		return err
	}
	defer conn.Close()

	client := collector.NewCollectorClient(conn)
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	stream, err := client.Collect(ctx)
	if err != nil {
		return err
	}

	// Testing in local machine.
	if *local_test {
		return run_local_test(stream)
	}

	////////////////////////////////////////////////
	logmsg("Connect to k8s.")
	config, err := rest.InClusterConfig()
	if err != nil {
		return err
	}

	clientset, err := kubernetes.NewForConfig(config)
	if err != nil {
		return err
	}

	logmsg("Fetch k8s ReplicaSet info.")
	apps_api := clientset.AppsV1()
	rs_list, err := apps_api.ReplicaSets(metav1.NamespaceAll).List(ctx, metav1.ListOptions{})
	if err != nil {
		return err
	}

	for _, rs := range rs_list.Items {
		err := send_info(make_collector_info_from_replicaset(collector.Info_ADDED, make_rs_info(&rs)), stream)
		if err != nil {
			return err
		}
	}

	logmsg("Fetch k8s Pod info.")
	core_api := clientset.CoreV1()
	pod_list, err := core_api.Pods(metav1.NamespaceAll).List(ctx, metav1.ListOptions{})
	if err != nil {
		return err
	}

	for _, pod := range pod_list.Items {
		err := send_info(make_collector_info_from_pod(collector.Info_ADDED, make_pod_info(&pod)), stream)
		if err != nil {
			return err
		}
	}

	logmsg("Start watch.")

	cancel_ch := make(chan error)
	go func() {
		_, err := stream.Recv()
		if err != nil {
			cancel_ch <- err
		} else {
			cancel_ch <- errors.New("Relay signals reset")
		}
		close(cancel_ch)
	}()

	pod_version := pod_list.ListMeta.ResourceVersion
	rs_version := rs_list.ListMeta.ResourceVersion

	tick_ch := time.NewTicker(5 * time.Minute).C
	for {
		// Watch Pod
		pod_watcher, err := core_api.Pods(metav1.NamespaceAll).Watch(ctx,
			metav1.ListOptions{
				ResourceVersion: pod_version,
			})
		if err != nil {
			return err
		}
		pod_ch := pod_watcher.ResultChan()

		// Watch ReplicaSet
		rs_watcher, err := apps_api.ReplicaSets(metav1.NamespaceAll).Watch(ctx,
			metav1.ListOptions{
				ResourceVersion: rs_version,
			})
		if err != nil {
			pod_watcher.Stop()
			return err
		}
		rs_ch := rs_watcher.ResultChan()

		keep_watching := true
		for keep_watching {
			select {
			case event := <-rs_ch:
				version, err := handle_rs_event(event, stream)
				if err != nil {
					pod_watcher.Stop()
					rs_watcher.Stop()
					return err
				}
				rs_version = *version

			case event := <-pod_ch:
				version, err := handle_pod_event(event, stream)
				if err != nil {
					pod_watcher.Stop()
					rs_watcher.Stop()
					return err
				}
				pod_version = *version

			case <-ctx.Done():
				logmsg("server signals canceld")
				pod_watcher.Stop()
				rs_watcher.Stop()
				return ctx.Err()

			case err := <-cancel_ch:
				pod_watcher.Stop()
				rs_watcher.Stop()
				return err

			case <-tick_ch:
				keep_watching = false
				logmsg("end of one iteration of watch loop.")
				pod_watcher.Stop()
				rs_watcher.Stop()
			}
		}
	}
}

func main() {
	flag.Parse()
	for {
		err := run()
		if err != nil {
			logmsg(fmt.Sprintf("Error: %v", err))
		}
		time.Sleep(200 * time.Millisecond)
	}
}
