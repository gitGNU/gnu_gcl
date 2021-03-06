;; -*-Lisp-*-
;; Copyright (C) 1994 M. Hagiya, W. Schelter, T. Yuasa

;; This file is part of GNU Common Lisp, herein referred to as GCL
;;
;; GCL is free software; you can redistribute it and/or modify it under
;;  the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;; 
;; GCL is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
;; FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public 
;; License for more details.
;; 
;; You should have received a copy of the GNU Library General Public License 
;; along with GCL; see the file COPYING.  If not, write to the Free Software
;; Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.


;;;;    defmacro.lsp
;;;;
;;;;         defines SI:DEFMACRO*, the defmacro preprocessor


;; (in-package :lisp)
;; (export '(lambda defvar import &whole &environment &body))


(in-package :system)


;;; valid lambda-list to DEFMACRO is:
;;;
;;;	( [ &whole sym ]
;;;	  [ &environment sym ]
;;;	  { v }*
;;;	  [ &optional { sym | ( v [ init [ v ] ] ) }* ]
;;;	  {  [ { &rest | &body } v ]
;;;	     [ &key { sym | ( { sym | ( key v ) } [ init [ v ]] ) }*
;;;		    [ &allow-other-keys ]]
;;;	     [ &aux { sym | ( v [ init ] ) }* ]
;;;	  |  . sym }
;;;	 )
;;;
;;; where v is short for { defmacro-lambda-list | sym }.
;;; A symbol may be accepted as a DEFMACRO lambda-list, in which case
;;; (DEFMACRO <name> <symbol> ... ) is equivalent to
;;; (DEFMACRO <name> (&REST <symbol>) ...).
;;; Defamcro-lambda-list is defined as:
;;;
;;;	( { v }*
;;;	  [ &optional { sym | ( v [ init [ v ] ] ) }* ]
;;;	  {  [ { &rest | &body } v ]
;;;	     [ &key { sym | ( { sym | ( key v ) } [ init [ v ]] ) }*
;;;		    [ &allow-other-keys ]]
;;;	     [ &aux { sym | ( v [ init ] ) }* ]
;;;	  |  . sym }
;;;	 )

;; defvar is not yet available.
(mapc '*make-special '(*dl* *key-check* *arg-check*))


(defun get-&environment(vl &aux env)
  (let ((env-m
	 (and (listp vl)
	      (do ((tail vl (cdr tail)))
		  ((not (consp tail)) nil)
		(when (eq '&environment (car tail))
		  (return tail))))))
    (cond (env-m
 	   (setq env (cadr env-m))
 	   (setq vl (append (ldiff vl env-m) (cddr env-m)))))
    (values vl env)))



(defun gensym (&optional (x nil xp))
  (cond ((not xp) (gensym0))
	((stringp x) (gensym1s x))
	((gensym1ig x))))
			 
(export '(blocked-body-name parse-body-header))

(defun parse-body-header (x &optional doc decl ctps &aux (a (car x)))
  (cond 
   ((unless (or doc ctps) (and (stringp a) (cdr x))) (parse-body-header (cdr x) a decl ctps))
   ((unless ctps (when (consp a) (eq (car a) 'declare)))  (parse-body-header (cdr x) doc (cons a decl) ctps))
   ((when (consp a) (member (car a) '(check-type assert))) (parse-body-header (cdr x) doc decl (cons a ctps)))
   (t (values doc (nreverse decl) (nreverse ctps) x))))

(defun make-blocked-lambda (ll decls ctps body block)
  (let ((body (if (eq block (blocked-body-name body)) body `((block ,block ,@body)))))
    `(lambda ,ll ,@decls ,@ctps ,@body)))

(defun blocked-body-name (body)
  (when (and (not (cdr body))
	     (consp (car body))
	     (eq (caar body) 'block))
    (cadar body)))


(defun defmacro-lambda (name vl body &aux whole)

  (cond ((listp vl))
        ((symbolp vl) (setq vl (list '&rest vl)))
        ((error "The defmacro-lambda-list ~s is not a list." vl)))
  
  (cond ((and (listp vl) (eq (car vl) '&whole))
	 (setq whole (cadr vl)) (setq vl (cddr vl)))
	((setq whole (gensym))))  
  
  (multiple-value-bind
   (doc decls ctps body)
   (parse-body-header body)

   (declare (ignore doc))
   
   (multiple-value-bind
    (vl env)
    (get-&environment vl)
    
    (let* ((envp env)
	   (env (or env (gensym)))
	   (*dl* `(&aux ,env ,whole))
	   *key-check* *arg-check*
	   (ppn (dm-vl vl whole t)))

      (declare (ignore ppn))
      
      (dolist (kc *key-check*)
	(push `(unless (getf ,(car kc) :allow-other-keys);FIXME order?
		 (do ((vl ,(car kc) (cddr vl)))
		     ((endp vl))
		     (unless (member (car vl) ',(cons :allow-other-keys (cdr kc)))
		       (dm-key-not-allowed (car vl)))))
	      body))
      
      (dolist (ac *arg-check*)
	(push `(when ,(dm-nth-cdr (cdr ac) (car ac)) (dm-too-many-arguments)) body))

      (unless envp (push `(declare (ignore ,env)) decls))

      (make-blocked-lambda (nreverse *dl*) decls ctps body name)))))

;; (defun si:defmacro* (name vl body
;;                           &aux *dl* (*key-check* nil)
;; 			  (*arg-check* nil)
;; 			  doc decls whole ppn (env nil) envp)
;;   (cond ((listp vl))
;;         ((symbolp vl) (setq vl (list '&rest vl)))
;;         (t (error "The defmacro-lambda-list ~s is not a list." vl)))
;;   (multiple-value-setq (doc decls body) (find-doc body nil))
;;   (cond ((and (listp vl) (eq (car vl) '&whole))
;;          (setq whole (cadr vl)) (setq vl (cddr vl)))
;;         (t (setq whole (gensym))))
;;   (multiple-value-setq (vl env)
;; 		       (get-&environment vl))
;;   (setq envp env)
;;   (or env (setq env (gensym)))
;;   (setq *dl* `(&aux ,env ,whole))
;;   (setq ppn (dm-vl vl whole t))
;;   (dolist (kc *key-check*)
;;     (push `(unless (getf ,(car kc) :allow-other-keys);FIXME order?
;; 	     (do ((vl ,(car kc) (cddr vl)))
;; 		 ((endp vl))
;; 		 (unless (member (car vl) ',(cons :allow-other-keys (cdr kc)))
;; 		   (dm-key-not-allowed (car vl)))))
;; 	  body))
;;   (dolist (ac *arg-check*)
;;     (push `(when ,(dm-nth-cdr (cdr ac) (car ac))
;; 	     (dm-too-many-arguments)) body))
;;   (unless envp (push `(declare (ignore ,env)) decls))
;; ;  (list doc ppn `(lambda-block ,name ,(reverse *dl*) ,@(append decls body)))
;;   (list doc ppn (eval `(function (lambda ,(reverse *dl*) ,@decls (block ,name ,@body)))))
;; ;  (list doc ppn (let ((nn (gensym))) (eval `(defun ,nn ,(reverse *dl*) ,@decls (block ,name ,@body))) (symbol-function nn)))
;;   )

(defun dm-vl (vl whole top)
  (when (consp whole)
    (let ((n (gensym)))
      (setq *dl* (subst n whole *dl*))
      (dm-vl whole n nil)
      (setq whole n)))


 (do ((optionalp nil) (restp nil) (keyp nil)
       (allow-other-keys-p nil) (auxp nil)
       (rest nil) (allow-other-keys nil) (keys nil) (no-check nil)
       (n (if top 1 0)) (ppn nil)
       )
      ((not (consp vl))
       (when vl
         (when restp (dm-bad-key '&rest))
         (push (list vl (dm-nth-cdr n whole)) *dl*)
         (setq no-check t))
       (when (and rest (not allow-other-keys))
         (push (cons rest keys) *key-check*))
       (unless no-check (push (cons whole n) *arg-check*))
       ppn
       )
    (let ((v (car vl)))
      (cond
       ((eq v '&optional)
        (when optionalp (dm-bad-key '&optional))
        (setq optionalp t)
        (pop vl))
       ((or (eq v '&rest) (eq v '&body))
        (when restp (dm-bad-key v))
        (dm-v (cadr vl) (dm-nth-cdr n whole))
        (setq restp t optionalp t no-check t)
        (setq vl (cddr vl))
        (when (eq v '&body) (setq ppn (if top (1- n) n))))
       ((eq v '&key)
        (when keyp (dm-bad-key '&key))
        (setq rest (gensym))
        (push (list rest (dm-nth-cdr n whole)) *dl*)
        (setq keyp t restp t optionalp t no-check t)
        (pop vl))
       ((eq v '&allow-other-keys)
        (when (or (not keyp) allow-other-keys-p)
              (dm-bad-key '&allow-other-keys))
        (setq allow-other-keys-p t)
        (setq allow-other-keys t)
        (pop vl))
       ((eq v '&aux)
        (when auxp (dm-bad-key '&aux))
        (setq auxp t allow-other-keys-p t keyp t restp t optionalp t)
        (pop vl))
       (auxp
        (let (x (init nil))
             (cond ((symbolp v) (setq x v))
                   (t (setq x (car v))
                      (unless (endp (cdr v)) (setq init (cadr v)))))
             (dm-v x init))
        (pop vl))
       (keyp
        (let ((temp (gensym)) x k (init nil) (sv nil))
             (cond ((symbolp v) (setq x v k (intern (string v) 'keyword)))
                   (t (if (symbolp (car v))
                          (setq x (car v)
                                k (intern (string (car v)) 'keyword))
                          (setq x (cadar v) k (caar v)))
                      (unless (endp (cdr v))
                              (setq init (cadr v))
                              (unless (endp (cddr v))
                                      (setq sv (caddr v))))))
             (dm-v temp `(getf ,rest ,k 'failed))
             (dm-v x `(if (eq ,temp 'failed) ,init ,temp))
             (when sv (dm-v sv `(not (eq ,temp 'failed))))
             (push k keys))
        (pop vl))
       (optionalp
        (let (x (init nil) (sv nil))
             (cond ((symbolp v) (setq x v))
                   (t (setq x (car v))
                      (unless (endp (cdr v))
                              (setq init (cadr v))
                              (unless (endp (cddr v))
                                      (setq sv (caddr v))))))
             (dm-v x `(if ,(dm-nth-cdr n whole) ,(dm-nth n whole) ,init))
             (when sv (dm-v sv `(not (null ,(dm-nth-cdr n whole))))))
        (incf n)
        (pop vl)
        )
       (t (dm-v (or v (gensym)) `(if ,(dm-nth-cdr n whole)
                       ,(dm-nth n whole)
                       (dm-too-few-arguments)))
          (incf n)
          (pop vl))
       ))))

(defun dm-v (v init)
       (cond  ((symbolp v) (push (if init (list v init) v) *dl*))
	      ((consp v) (let* ((w (eq (car v) '&whole))
				(temp (if w (cadr v) (gensym)))
				(v (if w (cddr v) v)))
			   (push (if init (list temp init) temp) *dl*)
			   (dm-vl v temp nil)))
	      (t (error "Bad defmacro argument: ~s" v))))

(defun dm-nth (n v)
  (multiple-value-bind (q r) (floor n 4)
     (dotimes (i q) (setq v (list 'cddddr v)))
     (case r
        (0 (list 'car v))
        (1 (list 'cadr v))
        (2 (list 'caddr v))
        (3 (list 'cadddr v))
        )))

(defun dm-nth-cdr (n v)
  (multiple-value-bind (q r) (floor n 4)
     (dotimes (i q) (setq v (list 'cddddr v)))
     (case r
        (0 v)
        (1 (list 'cdr v))
        (2 (list 'cddr v))
        (3 (list 'cdddr v))
        )))

(defun dm-bad-key (key)
       (error 'program-error :format-control "Defmacro-lambda-list contains illegal use of ~s." :format-arguments (list key)))

(defun dm-too-few-arguments ()
       (error 'program-error :format-control "Too few arguments are supplied to defmacro-lambda-list."))

(defun dm-too-many-arguments ()
       (error 'program-error :format-control "Too many arguments are supplied to defmacro-lambda-list."))

(defun dm-key-not-allowed (key)
       (error 'program-error :format-control "The key ~s is not allowed." :format-arguments (list key)))

(defun find-declarations (body)
  (if (endp body)
      (values nil nil)
      (let ((d (macroexpand (car body))))
        (cond ((stringp d)
               (if (endp (cdr body))
                   (values nil (list d))
                   (multiple-value-bind (ds b)
                       (find-declarations (cdr body))
                     (values (cons d ds) b))))
              ((and (consp d) (eq (car d) 'declare))
               (multiple-value-bind (ds b)
                   (find-declarations (cdr body))
                 (values (cons d ds) b)))
              (t
               (values nil (cons d (cdr body))))))))

(defmacro symbol-to-function (sym)
  (let* ((n (gensym))
	 (gf (find-symbol "C-SYMBOL-GFDEF" (find-package :s))))
    `(when (symbolp ,sym)
       ,(if (fboundp gf) `(let ((,n (address (,gf ,sym))))
			    (unless (= +objnull+ ,n) (nani ,n)))
	  `(let* ((,n (when (fboundp ,sym) (symbol-function ,sym)))
		  (,n (if (and (consp ,n) (eq (car ,n) 'macro)) (cdr ,n) ,n)))
	     (unless (consp ,n) ,n))))))

(defmacro call (sym &optional f &rest keys) ;FIXME macro
  (let* ((fnf (gensym))(n (gensym)))
    `(let* ((,fnf (if (functionp ,sym) ,sym (symbol-to-function ,sym))));(coerce ,sym 'function)
       (or (when ,fnf (cfun-call ,fnf))
	   (when ,f
	     (let ((,n (make-call ,@keys)))
	       (when ,fnf (set-cfun-call ,n ,fnf))
	       ,n))))))
