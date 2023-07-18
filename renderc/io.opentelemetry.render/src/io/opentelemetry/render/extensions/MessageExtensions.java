// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0
package io.opentelemetry.render.extensions;

import com.google.common.base.Objects;
import io.opentelemetry.render.render.Field;
import io.opentelemetry.render.render.Message;
import io.opentelemetry.render.render.MessageType;
import io.opentelemetry.render.render.Span;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.xbase.lib.CollectionLiterals;
import org.eclipse.xtext.xbase.lib.Functions.Function1;
import org.eclipse.xtext.xbase.lib.IterableExtensions;
import org.eclipse.xtext.xbase.lib.ListExtensions;

public class MessageExtensions {
  private static String prependCommaIfNotEmpty(String string) {
    return  string.isEmpty() ? string : ", " + string;
  }

  public static String prototype(Message msg) {
    List<Field> sortedFields = msg.getFields().stream()
      .sorted(Comparator.comparingInt(Field::getId))
      .collect(Collectors.toList());

    String finalString = sortedFields.stream()
      .map( it -> it.getType().isIsShortString() ? "const char " + it.getName() + "[" +it.getType().getSize() + "]" + FieldExtensions.arraySuffix(it) : "const " + FieldTypeExtensions.parsedCType(it.getType()) + " " + it.getName() +  FieldExtensions.arraySuffix(it))
      .collect(Collectors.joining(","));
    return finalString;
  }

  public static String commaPrototype(Message msg) {
    return MessageExtensions.prependCommaIfNotEmpty(MessageExtensions.prototype(msg));
  }

  public static String callPrototype(Message msg) {
    String finalString = msg.getFields().stream()
      .sorted(Comparator.comparingInt(Field::getId))
      .map(Field::getName)
      .collect(Collectors.joining(","));    
    return finalString;
  }

  public static String commaCallPrototype(Message msg) {
    return MessageExtensions.prependCommaIfNotEmpty(MessageExtensions.callPrototype(msg));
  }

  public static String norefPrototype(Message msg) {
    String finalString = msg.getFields().stream()
      .filter( c -> c != msg.getReference_field())
      .sorted(Comparator.comparingInt(Field::getId))
      .map(it -> "const " + FieldTypeExtensions.parsedCType(it.getType()) +  " " + it.getName() + FieldExtensions.arraySuffix(it))
      .collect(Collectors.joining(","));    
    return finalString;
  }

  public static String norefCommaPrototype(Message msg) {
    String finalString = msg.getFields().stream()
      .filter( c -> c != msg.getReference_field() )
      .sorted(Comparator.comparingInt(Field::getId))
      .map(it -> "const " + FieldTypeExtensions.parsedCType(it.getType()) + " " + it.getName() + FieldExtensions.arraySuffix(it) )
      .collect(Collectors.joining(","));    
    return MessageExtensions.prependCommaIfNotEmpty(finalString);
  }

  public static String norefCommaCallPrototype(Message msg) {
    String finalString = msg.getFields().stream()
      .filter( c -> c != msg.getReference_field() )
      .sorted(Comparator.comparingInt(Field::getId))
      .map(it -> it.getType().isIsShortString() ? it.getName() + ".data()" : it.getName() )
      .collect(Collectors.joining(","));    
    return MessageExtensions.prependCommaIfNotEmpty(finalString);
  }

  public static Span span(final Message msg) {
    EObject eContainer = msg.eContainer();
    return ((Span) eContainer);
  }

  public static Set<String> errors(Message msg) {
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

