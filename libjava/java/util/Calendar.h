
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_Calendar__
#define __java_util_Calendar__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>


class java::util::Calendar : public ::java::lang::Object
{

  static ::java::util::ResourceBundle * getBundle(::java::util::Locale *);
public: // actually protected
  Calendar();
  Calendar(::java::util::TimeZone *, ::java::util::Locale *);
public:
  static ::java::util::Calendar * getInstance();
  static ::java::util::Calendar * getInstance(::java::util::TimeZone *);
  static ::java::util::Calendar * getInstance(::java::util::Locale *);
  static ::java::util::Calendar * getInstance(::java::util::TimeZone *, ::java::util::Locale *);
  static JArray< ::java::util::Locale * > * getAvailableLocales();
public: // actually protected
  virtual void computeTime() = 0;
  virtual void computeFields() = 0;
public:
  virtual ::java::util::Date * getTime();
  virtual void setTime(::java::util::Date *);
  virtual jlong getTimeInMillis();
  virtual void setTimeInMillis(jlong);
  virtual jint get(jint);
public: // actually protected
  virtual jint internalGet(jint);
public:
  virtual void set(jint, jint);
  virtual void set(jint, jint, jint);
  virtual void set(jint, jint, jint, jint, jint);
  virtual void set(jint, jint, jint, jint, jint, jint);
  virtual void clear();
  virtual void clear(jint);
  virtual jboolean isSet(jint);
public: // actually protected
  virtual void complete();
public:
  virtual jboolean equals(::java::lang::Object *);
  virtual jint hashCode();
  virtual jboolean before(::java::lang::Object *);
  virtual jboolean after(::java::lang::Object *);
  virtual void add(jint, jint) = 0;
  virtual void roll(jint, jboolean) = 0;
  virtual void roll(jint, jint);
  virtual void setTimeZone(::java::util::TimeZone *);
  virtual ::java::util::TimeZone * getTimeZone();
  virtual void setLenient(jboolean);
  virtual jboolean isLenient();
  virtual void setFirstDayOfWeek(jint);
  virtual jint getFirstDayOfWeek();
  virtual void setMinimalDaysInFirstWeek(jint);
  virtual jint getMinimalDaysInFirstWeek();
  virtual jint getMinimum(jint) = 0;
  virtual jint getMaximum(jint) = 0;
  virtual jint getGreatestMinimum(jint) = 0;
  virtual jint getLeastMaximum(jint) = 0;
  virtual jint getActualMinimum(jint);
  virtual jint getActualMaximum(jint);
  virtual jint Calendar$compareTo(::java::util::Calendar *);
  virtual ::java::lang::Object * clone();
  virtual ::java::lang::String * toString();
private:
  void writeObject(::java::io::ObjectOutputStream *);
  void readObject(::java::io::ObjectInputStream *);
public:
  virtual ::java::lang::String * getDisplayName(jint, jint, ::java::util::Locale *);
  virtual ::java::util::Map * getDisplayNames(jint, jint, ::java::util::Locale *);
  virtual jint compareTo(::java::lang::Object *);
  static const jint ERA = 0;
  static const jint YEAR = 1;
  static const jint MONTH = 2;
  static const jint WEEK_OF_YEAR = 3;
  static const jint WEEK_OF_MONTH = 4;
  static const jint DATE = 5;
  static const jint DAY_OF_MONTH = 5;
  static const jint DAY_OF_YEAR = 6;
  static const jint DAY_OF_WEEK = 7;
  static const jint DAY_OF_WEEK_IN_MONTH = 8;
  static const jint AM_PM = 9;
  static const jint HOUR = 10;
  static const jint HOUR_OF_DAY = 11;
  static const jint MINUTE = 12;
  static const jint SECOND = 13;
  static const jint MILLISECOND = 14;
  static const jint ZONE_OFFSET = 15;
  static const jint DST_OFFSET = 16;
  static const jint FIELD_COUNT = 17;
  static const jint SUNDAY = 1;
  static const jint MONDAY = 2;
  static const jint TUESDAY = 3;
  static const jint WEDNESDAY = 4;
  static const jint THURSDAY = 5;
  static const jint FRIDAY = 6;
  static const jint SATURDAY = 7;
  static const jint JANUARY = 0;
  static const jint FEBRUARY = 1;
  static const jint MARCH = 2;
  static const jint APRIL = 3;
  static const jint MAY = 4;
  static const jint JUNE = 5;
  static const jint JULY = 6;
  static const jint AUGUST = 7;
  static const jint SEPTEMBER = 8;
  static const jint OCTOBER = 9;
  static const jint NOVEMBER = 10;
  static const jint DECEMBER = 11;
  static const jint UNDECIMBER = 12;
  static const jint AM = 0;
  static const jint PM = 1;
  static const jint ALL_STYLES = 0;
  static const jint SHORT = 1;
  static const jint LONG = 2;
public: // actually protected
  JArray< jint > * __attribute__((aligned(__alignof__( ::java::lang::Object)))) fields;
  JArray< jboolean > * isSet__;
  jlong time;
  jboolean isTimeSet;
  jboolean areFieldsSet;
private:
  ::java::util::TimeZone * zone;
  static ::java::lang::String * calendarClassName;
  jboolean lenient;
  jint firstDayOfWeek;
  jint minimalDaysInFirstWeek;
  jboolean explicitDSTOffset;
  jint serialVersionOnStream;
public: // actually package-private
  static const jlong serialVersionUID = -1807547505821590642LL;
private:
  static ::java::lang::String * bundleName;
  static ::java::util::HashMap * cache;
  static JArray< ::java::lang::Class * > * ctorArgTypes;
  static JArray< ::java::lang::String * > * fieldNames;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_Calendar__
