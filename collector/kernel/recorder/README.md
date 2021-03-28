Goal: to run the kernel-collecor with a workload in a VM, and extract the raw message output both BPF-userspace and userspace-pipeline

# TLDR; using the debug docker image

When building a debug build with the makefile / build-env, the docker container will be ready to execute this mode by setting the environment variable `FLOWMILL_RUN_LOCAL` to non-empty. Recordings will be in `/trace`, so mount that directory to be able to access the traces on the running machine:

```bash
mkdir $HOME/trace
sudo docker run  \
     --env FLOWMILL_RUN_LOCAL="yes" \
     --privileged \
     --pid host \
     --network host \
     --volume /sys/fs/cgroup:/hostfs/sys/fs/cgroup \
     --volume /usr/src:/var/run/flowmill/host/usr/src \
     --volume /lib/modules:/var/run/flowmill/host/lib/modules \
     --volume /etc:/var/run/flowmill/host/etc \
     --volume /var/cache:/var/run/flowmill/host/cache \
     --volume /var/run/docker.sock:/var/run/docker.sock \
     --volume $HOME/trace:/trace \
     --name flowmill-agent \
     flowmill-agent
```

## Building with build-env

in `~/out`:
```bash
../build.sh --cmake --debug
make flowmill-agent_docker_image -j8
```

# Details: how this works

Plan: 
1. Create a CA and certificates that we can use on localhost to emulate authz and the pipeline ports.
1. Serve a fake authz token locally on port 4433.
1. Create a telemetry intake on port 4444 that dumps its output to file.
1. Run the agent with the `bpf_dump` feature.
1. Run our crafted, predictable workload.
1. Extract the raw messages, create JSON output, run consistency checks.

## Keys and certificates

There are keys in the `keys` directory: `minica.pem` is a root certificate, and `localhost/key.pem` and `localhost/cert.pem` are a key and certificate for localhost. See below for instructions if you need to regenerate those.

## Trusting the generated CA

On Amazon Linux 2, requires the `openssl-perl` package. Available without further install on the build-env.

We need to install trust in two locations. The default `/etc/ssl/certs` for our use of curl (to fetch authz), and in `/root/install/openssl/certs`, which is the default directory for our custom-compiled OpenSSL we use for telemetry.

```bash
sudo cp keys/minica.pem /etc/ssl/certs
sudo c_rehash /etc/ssl/certs
sudo mkdir -p /root/install/openssl/certs/
sudo cp keys/minica.pem /root/install/openssl/certs/
sudo c_rehash /root/install/openssl/certs/
# on Amazon Linux 2, had to also `sudo cp /etc/ssl/certs/minica.pem /etc/ssl/certs/ca-certificates.crt`
```

### Generating new keys (if keys included are insufficient)

This can be done with `minica`:
```bash
go get -u github.com/jsha/minica
~/go/bin/minica -domains localhost
```

Will create `minica.pem` in the current directory, and in the `localhost` directory `key.pem` and `cert.pem`


## Running the pseudo-authz webserver

The following response is sufficient for the agent to accept from the pseudo-authz server:
```
{"token":"XYZ","issuedAtS":"1590626143","expirationS":"1590626743"}
```

There is a mock python webserver that serves that payload in `mock_authz_server.py`, and an stunnel config `stunnel-authz.conf` that uses the minica certs to serve.

For telemetry, `stunnel-telemetry.conf` configures stunnel to listen on port 7001 and tunnel to port 7000. Then netcat writes to file.


```bash
./mock_authz_server.py &
stunnel stunnel-authz.conf &
stunnel stunnel-telemetry.conf &
nc -l 7000 > /tmp/telemetry.raw &
```

### A different server approach: manually running, with openssl

This would be useful if you want to see TLS details, or manually send some output for authz:
```bash
openssl s_server -key ~/minica/localhost/key.pem -cert ~/minica/localhost/cert.pem -accept 4433
```

## Running the agent
assuming it's in `~/agent`:
```bash
sudo FLOWMILL_INTAKE_HOST=localhost FLOWMILL_INTAKE_PORT=7001 FLOWMILL_INTAKE_NAME=localhost FLOWMILL_AUTH_KEY_ID=KFIIVU75RGHY7EC7JABW FLOWMILL_AUTH_SECRET=KFIIVU75RGHY7EC7JABW ~/agent --log-console --authz-server=https://localhost:8001 --bpf-dump-file=/tmp/bpf.raw
```

Note that the key ID is just a fabricated key ID of the right prefix and length, since the agent verifies those. The secret has to be non-empty, so we set it in this case to the same value as the key ID. These are never verified, since we're running a pseudo-authz service that returns a (bogus) token without validating the key.
