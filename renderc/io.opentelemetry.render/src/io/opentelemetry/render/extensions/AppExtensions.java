// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions;

import io.opentelemetry.render.render.App;
import io.opentelemetry.render.render.Document;
import io.opentelemetry.render.render.Message;
import io.opentelemetry.render.render.Metric;
import io.opentelemetry.render.render.PackageDefinition;
import io.opentelemetry.render.render.Span;
import java.util.List;
import java.util.Set;
import org.eclipse.emf.common.util.EList;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.xbase.lib.Functions.Function1;
import org.eclipse.xtext.xbase.lib.IterableExtensions;

public class AppExtensions {
  public static String hashName(final App app) {
    String name = app.getName();
    return (name + "_hash");
  }

  public static String hashFunctor(final App app) {
    String name = app.getName();
    return (name + "_hasher_t");
  }

  public static String hashSize(final App app) {
    String upperCase = AppExtensions.hashName(app).toUpperCase();
    return (upperCase + "_SIZE");
  }

  public static PackageDefinition pkg(final App app) {
    EObject eContainer = app.eContainer();
    return ((Document) eContainer).getPackage();
  }

  public static EList<Metric> metrics(final App app) {
    EObject eContainer = app.eContainer();
    return ((Document) eContainer).getMetrics();
  }

  public static Set<App> remoteApps(final App app) {
    final Function1<Span, Boolean> function = (Span it) -> {
      return it.isIsProxy();
    };
    final Function1<Span, App> function1 = (Span it) -> {
      return it.getRemoteApp();
    };
    return IterableExtensions.<App>toSet(IterableExtensions.<Span, App>map(IterableExtensions.<Span>filter(app.getSpans(), function), function1));
  }

  public static List<Message> messages(final App app) {
    final Function1<Span, EList<Message>> function = (Span it) -> {
      return it.getMessages();
    };
    return IterableExtensions.<Message>toList(IterableExtensions.<Span, Message>flatMap(app.getSpans(), function));
  }
}
