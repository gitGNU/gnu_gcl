;-*- Mode:     Lisp -*-
;;;; Author:   Paul Dietz
;;;; Created:  Sat Jun 14 05:45:21 2003
;;;; Contains: Tests of MAKE-SYMBOL

(in-package :cl-test)

(deftest make-symbol.1
  (notnot-mv (symbolp (make-symbol "FOO")))
  t)

(deftest make-symbol.2
  (symbol-package (make-symbol "BAR"))
  nil)

(deftest make-symbol.3
  (symbol-package (make-symbol "CL::FOO"))
  nil)

(deftest make-symbol.4
  (symbol-package (make-symbol "CL:FOO"))
  nil)

(deftest make-symbol.5
  (symbol-name (make-symbol "xyz"))
  "xyz")

(deftest make-symbol.6
    (eqt (make-symbol "A")
	(make-symbol "A"))
  nil)

(deftest make-symbol.7
  (boundp (make-symbol "B"))
  nil)

(deftest make-symbol.8
  (symbol-plist (make-symbol "C"))
  nil)

(deftest make-symbol.9
  (fboundp (make-symbol "D"))
  nil)

(deftest make-symbol.10
  (symbol-name (make-symbol ""))
  "")

(deftest make-symbol.11
  :notes (:nil-vectors-are-strings)
  (symbol-name (make-symbol (make-array '(0) :element-type nil)))
  "")

(deftest make-symbol.order.1
  (let ((i 0))
    (values
     (symbol-name (make-symbol (progn (incf i) "ABC")))
     i))
  "ABC" 1)

(deftest make-symbol.error.1
  (signals-error (make-symbol nil) type-error)
  t)

(deftest make-symbol.error.2
  (signals-error (make-symbol 'a) type-error)
  t)

(deftest make-symbol.error.3
  (signals-error (make-symbol 1) type-error)
  t)

(deftest make-symbol.error.4
  (signals-error (make-symbol -1) type-error)
  t)

(deftest make-symbol.error.5
  (signals-error (make-symbol 1.213) type-error)
  t)

(deftest make-symbol.error.6
  (signals-error (make-symbol -1312.2) type-error)
  t)

(deftest make-symbol.error.7
  (signals-error (make-symbol #\w) type-error)
  t)

(deftest make-symbol.error.8
  (signals-error (make-symbol '(a)) type-error)
  t)

(deftest make-symbol.error.9
  (signals-error (make-symbol) program-error)
  t)

(deftest make-symbol.error.10
  (signals-error (make-symbol "a" "a") program-error)
  t)

(deftest make-symbol.error.11
  (signals-error (make-symbol '(#\a #\b #\c))  type-error)
  t)
