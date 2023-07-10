// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0


package io.opentelemetry.render.extensions;

import com.google.common.base.Objects;
import io.opentelemetry.render.render.Field;
import io.opentelemetry.render.render.Message;
import io.opentelemetry.render.render.MessageType;
import io.opentelemetry.render.render.Span;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.xbase.lib.CollectionLiterals;
import org.eclipse.xtext.xbase.lib.Functions.Function1;
import org.eclipse.xtext.xbase.lib.IterableExtensions;
import org.eclipse.xtext.xbase.lib.ListExtensions;

public class MessageExtensions {
  private static String prependCommaIfNotEmpty(final String s) {
    String xIfExpression = null;
    boolean equals = Objects.equal(s, "");
    if (equals) {
      xIfExpression = s;
    } else {
      xIfExpression = (", " + s);
    }
    return xIfExpression;
  }

  public static String prototype(final Message msg) {
    String xBlockExpression = null;
    {
      final Function1<Field, Integer> function = (Field it) -> {
        return Integer.valueOf(it.getId());
      };
      final List<Field> fields = IterableExtensions.<Field, Integer>sortBy(msg.getFields(), function);
      final Function1<Field, String> function1 = (Field it) -> {
        String xIfExpression = null;
        boolean isShortString = it.getType().isIsShortString();
        if (isShortString) {
          StringBuilder builder = new StringBuilder();
          builder.append("const char ");
          String name = it.getName();
          builder.append(name);
          builder.append("[");
          int size = it.getType().getSize();
          builder.append(size);
          builder.append("]");
          CharSequence arraySuffix = FieldExtensions.arraySuffix(it);
          builder.append(arraySuffix);
          xIfExpression = builder.toString();
        } else {
          StringBuilder builder1 = new StringBuilder();
          builder1.append("const ");
          String parsedCType = FieldTypeExtensions.parsedCType(it.getType());
          builder1.append(parsedCType);
          builder1.append(" ");
          String name1 = it.getName();
          builder1.append(name1);
          CharSequence _arraySuffix_1 = FieldExtensions.arraySuffix(it);
          builder1.append(_arraySuffix_1);
          xIfExpression = builder1.toString();
        }
        return xIfExpression;
      };
      final List<String> strings = ListExtensions.<Field, String>map(fields, function1);
      xBlockExpression = IterableExtensions.join(strings, ", ");
    }
    return xBlockExpression;
  }

public static String commaPrototype(final Message msg) {
    return MessageExtensions.prependCommaIfNotEmpty(MessageExtensions.prototype(msg));
  }

  public static String callPrototype(final Message msg) {
    final Function1<Field, Integer> function = (Field it) -> {
      return Integer.valueOf(it.getId());
    };
    final Function1<Field, String> function1 = (Field it) -> {
      return it.getName();
    };
    return IterableExtensions.join(ListExtensions.<Field, String>map(IterableExtensions.<Field, Integer>sortBy(msg.getFields(), function), function1), ", ");
  }

  public static String commaCallPrototype(final Message msg) {
    return MessageExtensions.prependCommaIfNotEmpty(MessageExtensions.callPrototype(msg));
  }

  public static String norefPrototype(final Message msg) {
    String xBlockExpression = null;
    {
      final Function1<Field, Boolean> function = (Field field) -> {
        Field referenceField = msg.getReference_field();
        return Boolean.valueOf((field != referenceField));
      };
      final Function1<Field, Integer> function1 = (Field it) -> {
        return Integer.valueOf(it.getId());
      };
      final List<Field> fields = IterableExtensions.<Field, Integer>sortBy(IterableExtensions.<Field>filter(msg.getFields(), function), function1);
      final Function1<Field, String> function2 = (Field it) -> {
        StringBuilder builder = new StringBuilder();
        builder.append("const ");
        String parsedCType = FieldTypeExtensions.parsedCType(it.getType());
        builder.append(parsedCType);
        builder.append(" ");
        String name = it.getName();
        builder.append(name);
        CharSequence _arraySuffix = FieldExtensions.arraySuffix(it);
        builder.append(_arraySuffix);
        return builder.toString();
      };
      final List<String> strings = ListExtensions.<Field, String>map(fields, function2);
      xBlockExpression = IterableExtensions.join(strings, ", ");
    }
    return xBlockExpression;
  }

  public static String norefCommaPrototype(final Message msg) {
    String xBlockExpression = null;
    {
      final Function1<Field, Boolean> function = (Field field) -> {
        Field referenceField = msg.getReference_field();
        return Boolean.valueOf((field != referenceField));
      };
      final Function1<Field, Integer> function1 = (Field it) -> {
        return Integer.valueOf(it.getId());
      };
      final List<Field> fields = IterableExtensions.<Field, Integer>sortBy(IterableExtensions.<Field>filter(msg.getFields(), function), function1);
      final Function1<Field, String> function2 = (Field it) -> {
        StringBuilder builder = new StringBuilder();
        builder.append("const ");
        String parsedCType = FieldTypeExtensions.parsedCType(it.getType());
        builder.append(parsedCType);
        builder.append(" ");
        String name = it.getName();
        builder.append(name);
        CharSequence _arraySuffix = FieldExtensions.arraySuffix(it);
        builder.append(_arraySuffix);
        return builder.toString();
      };
      final List<String> strings = ListExtensions.<Field, String>map(fields, function2);
      final String str = IterableExtensions.join(strings, ", ");
      xBlockExpression = MessageExtensions.prependCommaIfNotEmpty(str);
    }
    return xBlockExpression;
  }

 public static String norefCommaCallPrototype(final Message msg) {
    String xBlockExpression = null;
    {
      final Function1<Field, Boolean> function = (Field field) -> {
        Field referenceField = msg.getReference_field();
        return Boolean.valueOf((field != referenceField));
      };
      final Function1<Field, Integer> function1 = (Field it) -> {
        return Integer.valueOf(it.getId());
      };
      final List<Field> fields = IterableExtensions.<Field, Integer>sortBy(IterableExtensions.<Field>filter(msg.getFields(), function), function1);
      final Function1<Field, String> function2 = (Field it) -> {
        String xIfExpression = null;
        boolean _isIsShortString = it.getType().isIsShortString();
        if (_isIsShortString) {
          StringBuilder builder = new StringBuilder();
          String name = it.getName();
          builder.append(name);
          builder.append(".data()");
          xIfExpression = builder.toString();
        } else {
          xIfExpression = it.getName();
        }
        return xIfExpression;
      };
      final List<String> strings = ListExtensions.<Field, String>map(fields, function2);
      final String str = IterableExtensions.join(strings, ", ");
      xBlockExpression = MessageExtensions.prependCommaIfNotEmpty(str);
    }
    return xBlockExpression;
  }

  public static Span span(final Message msg) {
    EObject eContainer = msg.eContainer();
    return ((Span) eContainer);
  }

  public static Set<String> errors(final Message msg) {
    Set<String> xIfExpression = null;
    MessageType type = msg.getType();
    boolean equals = Objects.equal(type, MessageType.START);
    if (equals) {
      xIfExpression = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet("span_alloc_failed", "span_insert_failed", "span_pool_full", "duplicate_ref"));
    } else {
      Set<String> xIfExpression1 = null;
      MessageType type1 = msg.getType();
      boolean equals1 = Objects.equal(type1, MessageType.END);
      if (equals1) {
        xIfExpression1 = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet("span_find_failed", "span_erase_failed"));
      } else {
        Set<String> xIfExpression2 = null;
        boolean isSingleton = MessageExtensions.span(msg).isIsSingleton();
        boolean _not = (!isSingleton);
        if (_not) {
          xIfExpression2 = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet("span_find_failed"));
        } else {
          xIfExpression2 = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet());
        }
        xIfExpression1 = xIfExpression2;
      }
      xIfExpression = xIfExpression1;
    }
    return xIfExpression;
  }
}
