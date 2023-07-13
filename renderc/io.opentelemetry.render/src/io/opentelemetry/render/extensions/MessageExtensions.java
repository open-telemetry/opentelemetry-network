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
    String expression = s.isEmpty() ? s : ", " +s ;
    return expression;
  }

  public static String prototype(final Message msg) {
    String blockExpression = null;
    final Function1<Field, Integer> function = (Field it) -> {
      return Integer.valueOf(it.getId());
    };
    final List<Field> fields = IterableExtensions.<Field, Integer>sortBy(msg.getFields(), function);
    final Function1<Field, String> function1 = (Field it) -> {
      String expression = it.getType().isIsShortString() ? "const char " + it.getName() + "[" +it.getType().getSize() + "]" + FieldExtensions.arraySuffix(it) : "const " + FieldTypeExtensions.parsedCType(it.getType()) + " " + it.getName() +  FieldExtensions.arraySuffix(it);
      return expression;
    };
    final List<String> strings = ListExtensions.<Field, String>map(fields, function1);
    blockExpression = IterableExtensions.join(strings, ", ");
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
    final Function1<Field, Boolean> function = (Field field) -> {
      Field referenceField = msg.getReference_field();
      return Boolean.valueOf((field != referenceField));
    };
    final Function1<Field, Integer> function1 = (Field it) -> {
      return Integer.valueOf(it.getId());
    };
    final List<Field> fields = IterableExtensions.<Field, Integer>sortBy(IterableExtensions.<Field>filter(msg.getFields(), function), function1);
    final Function1<Field, String> function2 = (Field it) -> {
      return "const " + FieldTypeExtensions.parsedCType(it.getType()) +  " " + it.getName() + FieldExtensions.arraySuffix(it);
    };
    final List<String> strings = ListExtensions.<Field, String>map(fields, function2);
    blockExpression = IterableExtensions.join(strings, ", ");
    return blockExpression;
  }

  public static String norefCommaPrototype(final Message msg) {
    String blockExpression = null;
    final Function1<Field, Boolean> function = (Field field) -> {
      Field referenceField = msg.getReference_field();
      return Boolean.valueOf((field != referenceField));
    };
    final Function1<Field, Integer> function1 = (Field it) -> {
      return Integer.valueOf(it.getId());
    };
    final List<Field> fields = IterableExtensions.<Field, Integer>sortBy(IterableExtensions.<Field>filter(msg.getFields(), function), function1);
    final Function1<Field, String> function2 = (Field it) -> {
      return "const " + FieldTypeExtensions.parsedCType(it.getType()) + " " + it.getName() + FieldExtensions.arraySuffix(it) ;
    };
    final List<String> strings = ListExtensions.<Field, String>map(fields, function2);
    final String str = IterableExtensions.join(strings, ", ");
    blockExpression = MessageExtensions.prependCommaIfNotEmpty(str);
    return blockExpression;
  }

 public static String norefCommaCallPrototype(final Message msg) {
    String blockExpression = null;
    final Function1<Field, Boolean> function = (Field field) -> {
      return field != msg.getReference_field();
    };
    final Function1<Field, Integer> function1 = (Field it) -> {
      return Integer.valueOf(it.getId());
    };
    final List<Field> fields = IterableExtensions.<Field, Integer>sortBy(IterableExtensions.<Field>filter(msg.getFields(), function), function1);
    final Function1<Field, String> function2 = (Field it) -> {
      return it.getType().isIsShortString() ? it.getName() + ".data()" : it.getName();
    };
    final List<String> strings = ListExtensions.<Field, String>map(fields, function2);
    final String string = IterableExtensions.join(strings, ", ");
    blockExpression = MessageExtensions.prependCommaIfNotEmpty(string);
    return blockExpression;
  }

  public static Span span(final Message msg) {
    EObject eContainer = msg.eContainer();
    return ((Span) eContainer);
  }

  public static Set<String> errors(final Message msg) {
    Set<String> expression = null;
    MessageType type = msg.getType();
    if ( msg.getType() == MessageType.START) {
      expression = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet("span_alloc_failed", "span_insert_failed", "span_pool_full", "duplicate_ref"));
    } else {
      Set<String> tempExpression = null;
      if ( msg.getType() == MessageType.END) {
        tempExpression = Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet("span_find_failed", "span_erase_failed"));
      } else {
        tempExpression = !MessageExtensions.span(msg).isIsSingleton() ? Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet("span_find_failed")): Collections.<String>unmodifiableSet(CollectionLiterals.<String>newHashSet());
      }
      expression = tempExpression;
    }
    return expression;
  }
}
