;-*- Mode:     Lisp -*-
;;;; Author:   Paul Dietz
;;;; Created:  Sat Dec  6 14:45:16 2003
;;;; Contains: Tests for PATHNAME-VERSION

(in-package :cl-test)

(deftest pathname-version.1
  (loop for p in *pathnames*
	for version = (pathname-version p)
	unless (or (integerp version) (symbolp version))
	collect (list p version))
  nil)

(deftest pathname-version.error.1
  (classify-error (pathname-version))
  program-error)

(deftest pathname-version.error.2
  (classify-error (pathname-version *default-pathname-defaults* nil))
  program-error)

(deftest pathname-version.error.3
  (loop for x in *mini-universe*
	unless (or (could-be-pathname-designator x)
		   (handler-case (progn (pathname-version x) nil)
				 (type-error () t)
				 (condition () nil)))
	collect x)
  nil)

