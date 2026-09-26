// Copyright (c) 2005,2006,2007,2009,2010,2011 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Ron Wein          <wein@post.tau.ac.il>

#ifndef CGAL_ARRANGEMENT_2_ITERATORS_H
#define CGAL_ARRANGEMENT_2_ITERATORS_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#include <functional>

/*! \file
 * Definitions of auxiliary iterator adaptors.
 */

namespace CGAL {

// Arrangement handle types derive from the DCEL records but the DCEL
// allocates the base records; the downcast below is formally UB but
// layout-identical. See https://github.com/CGAL/cgal/issues/9140
#if defined(__has_attribute)
#  if __has_attribute(no_sanitize)
#    define CGAL_AOS2_NO_SANITIZE_VPTR __attribute__((no_sanitize("vptr")))
#  endif
#endif
#ifndef CGAL_AOS2_NO_SANITIZE_VPTR
#  define CGAL_AOS2_NO_SANITIZE_VPTR
#endif

/*! \class
 * An iterator adaptor for dereferencing the value-type of the iterator class
 * (given as Iterator_), which is supposed to be a pointer, and handle it as
 * the value-type given by Value_.
 */
template <typename Iterator_, typename Value_, typename Diff_, typename Category_>
class I_Dereference_iterator {
public:
  // Type definitions:
  using Iterator = Iterator_;

  using iterator_category = Category_;
  using value_type = Value_;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = Diff_;

  using Self = I_Dereference_iterator<Iterator, value_type, difference_type, iterator_category>;

protected:

  Iterator iter;           // The internal iterator.

public:
  /// \name Construction
  //@{
  I_Dereference_iterator() {}

  I_Dereference_iterator(Iterator it) : iter(it) {}
  //@}

  /// \name Basic operations.
  //@{
  bool operator==(const Self& it) const { return (iter == it.iter); }

  bool operator!=(const Self& it) const { return (!(iter == it.iter)); }

  Iterator current_iterator() const { return (iter); }

  pointer ptr() const { return (static_cast<value_type*>(*iter)); }

  reference operator*() const { return (*(ptr())); }

  pointer operator->() const { return (ptr()); }
  //@}

  /// \name Incremernt operations (forward category).
  //@{
  Self& operator++() {
    ++iter;
    return (*this);
  }

  Self operator++(int) {
    Self tmp = *this;
    ++iter;
    return (tmp);
  }
  //@}

  /// \name Decremernt operations (bidirectional category).
  //@{
  Self& operator--() {
    --iter;
    return (*this);
  }

  Self operator--(int) {
    Self tmp = *this;
    --iter;
    return (tmp);
  }
  //@}
};

/*! \class
 * An iterator adaptor for dereferencing the value-type of the const iterator
 * class (given as CIterator_), which is supposed to be a pointer, and handle
 * it as the value-type given by Value_.
 */
template <typename CIterator_, typename MIterator_, typename Value_, typename Diff_, typename Category_>
class I_Dereference_const_iterator {
public:
  // Type definitions:
  using Const_iterator = CIterator_;
  using Mutable_iterator = MIterator_;
  using Self = I_Dereference_const_iterator<CIterator_, MIterator_, Value_, Diff_, Category_>;

  using iterator_category = Category_;
  using value_type = Value_;
  using reference = const value_type&;
  using pointer = const value_type*;
  using difference_type = Diff_;

protected:
  Const_iterator iter;           // The internal iterator.

public:
  /// \name Construction
  //@{
  I_Dereference_const_iterator() {}

  I_Dereference_const_iterator(Const_iterator it) : iter(it) {}

  I_Dereference_const_iterator(Mutable_iterator it) : iter(Const_iterator(&(*it))) {}

  //@}

  /// \name Basic operations.
  //@{
  bool operator==(const Self& it) const { return (iter == it.iter); }

  bool operator!=(const Self& it) const { return (iter != it.iter); }

  Const_iterator current_iterator() const { return (iter); }

  pointer ptr() const { return (static_cast<const value_type*>(*iter)); }

  reference operator*() const { return (*(ptr())); }

  pointer operator->() const { return (ptr()); }
  //@}

  /// \name Incremernt operations (forward category).
  //@{
  Self& operator++() {
    ++iter;
    return (*this);
  }

  Self operator++(int) {
    Self tmp = *this;
    ++iter;
    return (tmp);
  }
  //@}

  /// \name Decremernt operations (bidirectional category).
  //@{
  Self& operator--() {
    --iter;
    return (*this);
  }

  Self operator--(int) {
    Self tmp = *this;
    --iter;
    return (tmp);
  }
  //@}
};

/*! \class
 * An iterator adaptor for the filtering a DCEL iterator (given as Iterator_)
 * using a given filter functor (Filter_).
 */
template <typename Iterator_, typename Filter_, typename Value_, typename Diff_, typename Category_>
class I_Filtered_iterator {
public:
  using Iterator = Iterator_;
  using Filter = Filter_;

  using iterator_category = Category_;
  using value_type = Value_;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = Diff_;

  using Self = I_Filtered_iterator<Iterator, Filter, value_type, difference_type, iterator_category>;

protected:
  Iterator nt;          // The internal iterator (this member should not
                        // be renamed in order to comply with the
                        // HalfedgeDS circulators that refer to it).
  Iterator iend;        // A past-the-end iterator.
  Filter filt;          // The filter functor.

public:
  /*! Constructors. */
  I_Filtered_iterator() {}

  I_Filtered_iterator(Iterator it) : nt(it), iend(nt) {}

  template <typename T>
  I_Filtered_iterator(T* p) : nt(p), iend(nt) {}

  I_Filtered_iterator(Iterator it, Iterator end) :
    nt(it),
    iend(end)
  { while (nt != iend && ! filt(*nt)) ++nt; }

  I_Filtered_iterator(Iterator it, Iterator end, Filter f) :
    nt(it),
    iend(end),
    filt(f)
  { while (nt != iend && ! filt(*nt)) ++nt; }

  template <typename P>
  I_Filtered_iterator& operator=(const P* p) {
    nt = pointer(p);
    iend =nt;
    return *this;
  }

  /*! Access operations. */
  Iterator current_iterator() const { return (nt); }

  Iterator past_the_end() const { return (iend); }

  Filter filter() const { return (filt); }

  CGAL_AOS2_NO_SANITIZE_VPTR
  pointer ptr() const { return static_cast<pointer>(&(*nt)); }

  /*! Equality operators. */
  bool operator==(const Self& it) const { return (nt == it.nt); }

  bool operator!=(const Self& it) const { return !(*this == it); }

  bool operator<(const Self& it) const { return &(**this) < (&*it); }

  /*! Dereferencing operators. */
  reference operator*() const { return (*(ptr())); }

  pointer operator->() const { return ptr(); }

  /*! Increment operators. */
  Self& operator++() {
    do ++nt;
    while (!(nt == iend) && ! filt(*nt));
    return (*this);
  }

  Self operator++(int) {
    Self tmp = *this;
    ++(*this);
    return tmp;
  }

  /*! Decrement operators. */
  Self& operator--() {
    do --nt;
    while (!(nt == iend) && ! filt(*nt));
    return (*this);
  }

  Self operator--(int) {
    Self tmp = *this;
    --(*this);
    return tmp;
  }
};

/*! \class
 * An iterator adaptor for the filtering a DCEL const iterator (given as
 * CIterator_) using a given filter functor (Filter_).
 */
template <typename CIterator_, typename Filter_, typename MIterator_, typename Value_, typename Diff_,
          typename Category_>
class I_Filtered_const_iterator {
public:
  using Iterator = CIterator_;
  using Filter = Filter_;

  using iterator_category = Category_;
  using value_type = Value_;
  using reference = const value_type&;
  using pointer = const value_type*;
  using difference_type = Diff_;

  using Self = I_Filtered_const_iterator<Iterator, Filter, MIterator_, value_type, difference_type, iterator_category>;
  using mutable_iterator = I_Filtered_iterator<MIterator_, Filter, value_type, difference_type, iterator_category>;

protected:
  Iterator nt;          // The internal iterator (this member should not
                        // be renamed in order to comply with the
                        // HalfedgeDS circulators that refer to it).
  Iterator iend;        // A past-the-end iterator.
  Filter filt;          // The filter functor.

public:
  /*! Constructors. */
  I_Filtered_const_iterator() {}

  I_Filtered_const_iterator(Iterator it) : nt(it), iend(it) {}

  template <typename T>
  I_Filtered_const_iterator(T* p) : nt(pointer(p)), iend(nt) {}

  I_Filtered_const_iterator(Iterator it, Iterator end) :
    nt(it),
    iend(end)
  { while (nt != iend && ! filt(*nt)) ++nt; }

  I_Filtered_const_iterator(Iterator it, Iterator end, Filter f) :
    nt(it),
    iend(end),
    filt(f)
  { while (nt != iend && ! filt(*nt)) ++nt; }

  I_Filtered_const_iterator(mutable_iterator it) :
    nt(it.current_iterator()),
    iend(it.past_the_end()),
    filt(it.filter())
  { /* while (nt != iend && ! filt (*nt)) ++nt; */ }

  template <typename P>
  I_Filtered_const_iterator& operator=(const P* p) {
    nt = pointer(p);
    iend =nt;
    return *this;
  }

  /*! Access operations. */
  Iterator current_iterator() const { return (nt); }

  Iterator past_the_end() const { return (iend); }

  Filter filter() const { return (filt); }

  pointer ptr() const { return static_cast<pointer>(&(*nt)); }

  /*! Equality operators. */
  bool operator==(const Self& it) const { return (nt == it.nt); }

  bool operator!=(const Self& it) const { return !(*this == it); }

  bool operator<(const Self& it) const { return &(**this) < (&*it); }

  /*! Dereferencing operators. */
  reference operator*() const { return (*(ptr())); }

  pointer operator->() const { return ptr(); }

  /*! Increment operators. */
  Self& operator++() {
    do ++nt;
    while (!(nt == iend) && ! filt(*nt));
    return (*this);
  }

  Self operator++(int) {
    Self tmp = *this;
    ++(*this);
    return tmp;
  }

  /*! Decrement operators. */
  Self& operator--() {
    do --nt;
    while (!(nt == iend) && ! filt (*nt));
    return (*this);
  }

  Self operator--(int) {
    Self tmp = *this;
    --(*this);
    return tmp;
  }
};

} //namespace CGAL

namespace std {

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4099) // For VC10 it is class hash
#endif

#ifndef CGAL_CFG_NO_STD_HASH

template <typename CIterator_, typename Filter_, typename MIterator_, typename Value_, typename Diff_,
          typename Category_>
struct hash<CGAL::I_Filtered_const_iterator<CIterator_, Filter_, MIterator_, Value_, Diff_, Category_>> {
  using I = CGAL::I_Filtered_const_iterator<CIterator_, Filter_, MIterator_, Value_, Diff_, Category_>;

  std::size_t operator()(const I& i) const { return reinterpret_cast<std::size_t>(&*i) / sizeof(Value_); }
};

template <typename Iterator_, typename Filter_, typename Value_, typename Diff_, typename Category_>
struct hash<CGAL::I_Filtered_iterator<Iterator_, Filter_, Value_, Diff_, Category_>> {
  using I = CGAL::I_Filtered_iterator<Iterator_, Filter_, Value_, Diff_, Category_>;

  std::size_t operator()(const I& i) const
  { return reinterpret_cast<std::size_t>(&*i) / sizeof(typename I::value_type); }
};

#endif

#if defined(BOOST_MSVC)
#pragma warning(pop)
#endif

} // namespace std

namespace  boost {

template <typename T> struct hash;

template <typename CIterator_, typename Filter_, typename MIterator_, typename Value_, typename Diff_,
          typename Category_>
struct hash<CGAL::I_Filtered_const_iterator<CIterator_, Filter_, MIterator_, Value_, Diff_, Category_>> {

  using I = CGAL::I_Filtered_const_iterator<CIterator_, Filter_, MIterator_, Value_, Diff_, Category_>;

  std::size_t operator()(const I& i) const { return reinterpret_cast<std::size_t>(&*i) / sizeof(Value_); }
};

template <typename Iterator_, typename Filter_, typename Value_, typename Diff_, typename Category_>
struct hash<CGAL::I_Filtered_iterator<Iterator_, Filter_, Value_, Diff_, Category_>> {
  using I = CGAL::I_Filtered_iterator<Iterator_, Filter_, Value_, Diff_, Category_>;

  std::size_t operator()(const I& i) const
  { return reinterpret_cast<std::size_t>(&*i) / sizeof(typename I::value_type); }
};

} // namespace boost

#endif
