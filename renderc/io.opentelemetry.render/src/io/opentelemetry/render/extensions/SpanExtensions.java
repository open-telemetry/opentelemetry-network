// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0


package io.opentelemetry.render.extensions;

import com.google.common.base.Objects;
import io.opentelemetry.render.render.App;
import io.opentelemetry.render.render.FieldType;
import io.opentelemetry.render.render.Message;
import io.opentelemetry.render.render.MessageType;
import io.opentelemetry.render.render.Span;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.xbase.lib.Conversions;
import org.eclipse.xtext.xbase.lib.Functions.Function1;
import org.eclipse.xtext.xbase.lib.IterableExtensions;

@SuppressWarnings("all")
public class SpanExtensions {
  public static String getBaseClassName(final Span span) {
    String camelCase = UtilityExtensions.toCamelCase(span.getName());
    return (camelCase + "SpanBase");
  }

  public static String getClassTypeName(final Span span) {
    String name = span.getName();
    return (name + "_type");
  }

  public static String getInstanceName(final Span span) {
    String name = span.getName();
    return (name + "__instance");
  }

  public static String fixedHashName(final Span span) {
    String name = span.getName();
    return (name + "__hash");
  }

  public static String fixedHashTypeName(final Span span) {
    String name = span.getName();
    return (name + "__hash_t");
  }

  public static String fixedHashHasherName(final Span span) {
    String name = span.getName();
    return (name + "__hasher_t");
  }

  public static Message proxyStartMessage(final Span span) {
    final Function1<Message, Boolean> _function = (Message it) -> {
      return Boolean.valueOf((Objects.equal(it.getType(), MessageType.START) && it.isReferenceEmbedded()));
    };
    final Message msg = IterableExtensions.<Message>findFirst(span.getRemoteSpan().getMessages(), _function);
    if ((msg == null)) {
      Span remoteSpan = span.getRemoteSpan();
      String plus = ("proxy: no viable start message: " + remoteSpan);
      throw new RuntimeException(plus);
    }
    return msg;
  }

 public static Message proxyEndMessage(final Span span) {
    final Function1<Message, Boolean> _function = (Message it) -> {
      return Boolean.valueOf(((Objects.equal(it.getType(), MessageType.END) && (((Object[])Conversions.unwrapArray(it.getFields(), Object.class)).length == 1)) && it.isReferenceEmbedded()));
    };
    final Message msg = IterableExtensions.<Message>findFirst(span.getRemoteSpan().getMessages(), _function);
    if ((msg == null)) {
      Span remoteSpan = span.getRemoteSpan();
      String plus = ("proxy: no viable end message: " + remoteSpan);
      throw new RuntimeException(plus);
    }
    return msg;
  }

  public static Iterable<Message> proxyLogMessages(final Span span) {
    final Function1<Message, Boolean> _function = (Message it) -> {
      return Boolean.valueOf(((Objects.equal(it.getType(), MessageType.LOG) || Objects.equal(it.getType(), MessageType.MSG)) && it.isReferenceEmbedded()));
    };
    return IterableExtensions.<Message>filter(span.getRemoteSpan().getMessages(), _function);
  }

  public static App app(final Span span) {
    EObject eContainer = span.eContainer();
    return ((App) eContainer);
  }

  public static FieldType referenceType(final Span span) {
    FieldType xBlockExpression = null;
    {
      final Function1<Message, Boolean> _function = (Message it) -> {
        return Boolean.valueOf(it.isReferenceEmbedded());
      };
      final Message messageWithRef = IterableExtensions.<Message>findFirst(span.getMessages(), _function);
      if ((messageWithRef == null)) {
        throw new RuntimeException("referenceType(span): span does not have any messages with reference_field");
      }
      xBlockExpression = messageWithRef.getReference_field().getType();
    }
    return xBlockExpression;
  }

  public static int pool_size(final Span span) {
    int poolSize = span.getPool_size_();
    boolean greaterThan = (poolSize > 0);
    if (greaterThan) {
      return span.getPool_size_();
    } else {
      return 4096;
    }
  }

  public static boolean conn_hash(final Span span) {
    boolean isConnHash = span.isConn_hash_();
    if (isConnHash) {
      return span.isConn_hash_();
    }
    return ((((Object[])Conversions.unwrapArray(span.getMessages(), Object.class)).length > 0) && (!span.isIsSingleton()));
  }
}
