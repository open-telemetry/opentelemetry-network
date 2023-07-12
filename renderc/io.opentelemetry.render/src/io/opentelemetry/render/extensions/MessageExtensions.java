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
    String expression = null;
    if (s.isEmpty()) {
      expression = s;
    } else {
      expression = (", " + s);
    }
    return expression;
  }

  public static String prototype(final Message msg) {
    String blockExpression = null;
    {
      final Function1<Field, Integer> function = (Field it) -> {
        return Integer.valueOf(it.getId());
      };
      final List<Field> fields = IterableExtensions.<Field, Integer>sortBy(msg.getFields(), function);
      final Function1<Field, String> function1 = (Field it) -> {
        String expression = null;
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
          expression = builder.toString();
        } else {
          StringBuilder builder1 = new StringBuilder();
          builder1.append("const ");
          String parsedCType = FieldTypeExtensions.parsedCType(it.getType());
          builder1.append(parsedCType);
          builder1.append(" ");
          String name1 = it.getName();
          builder1.append(name1);
          CharSequence arraySuffix1 = FieldExtensions.arraySuffix(it);
          builder1.append(arraySuffix1);
          expression = builder1.toString();
        }
        return expression;
      };
      final List<String> strings = ListExtensions.<Field, String>map(fields, function1);
      blockExpression = IterableExtensions.join(strings, ", ");
    }
    return blockExpression;
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
    String blockExpression = null;
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
        CharSequence arraySuffix = FieldExtensions.arraySuffix(it);
        builder.append(arraySuffix);
        return builder.toString();
      };
      final List<String> strings = ListExtensions.<Field, String>map(fields, function2);
      blockExpression = IterableExtensions.join(strings, ", ");
    }
    return blockExpression;
  }

  public static String norefCommaPrototype(final Message msg) {
    String blockExpression = null;
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
        CharSequence arraySuffix = FieldExtensions.arraySuffix(it);
        builder.append(arraySuffix);
        return builder.toString();
      };
      final List<String> strings = ListExtensions.<Field, String>map(fields, function2);
      final String str = IterableExtensions.join(strings, ", ");
      blockExpression = MessageExtensions.prependCommaIfNotEmpty(str);
    }
    return blockExpression;
  }

 public static String norefCommaCallPrototype(final Message msg) {
    String blockExpression = null;
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
        String expression = null;
        boolean isShortString = it.getType().isIsShortString();
        if (isShortString) {
          StringBuilder builder = new StringBuilder();
          String name = it.getName();
          builder.append(name);
          builder.append(".data()");
          expression = builder.toString();
        } else {
          expression = it.getName();
        }
        return expression;
      };
      final List<String> strings = ListExtensions.<Field, String>map(fields, function2);
      final String string = IterableExtensions.join(strings, ", ");
      blockExpression = MessageExtensions.prependCommaIfNotEmpty(string);
    }
    return blockExpression;
  }

  public static Span span(final Message msg) {
    EObject eContainer = msg.eContainer();
    return ((Span) eContainer);
  }

  public static Set<String> errors(final Message msg) {
    Set<String> expression = null;
    MessageType type = msg.getType();
    if (type == MessageType.START) {
      expression = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet("span_alloc_failed", "span_insert_failed", "span_pool_full", "duplicate_ref"));
    } else {
      Set<String> expression1 = null;
      MessageType type1 = msg.getType();
      if (type1 == MessageType.END) {
        expression1 = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet("span_find_failed", "span_erase_failed"));
      } else {
        Set<String> expression2 = null;
        boolean isSingleton = MessageExtensions.span(msg).isIsSingleton();
        
        if (!isSingleton) {
          expression2 = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet("span_find_failed"));
        } else {
          expression2 = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet());
        }
        expression1 = expression2;
      }
      expression = expression1;
    }
    return expression;
  }
}
