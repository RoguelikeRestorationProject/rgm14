;;; mark.ml: Rog-O-Matic XIV (CMU) Fri Dec 28 19:07:44 1984 - mlm

(defun
    (mark-rgm-edit buffer-was-modified
	(setq buffer-was-modified buffer-is-modified)
	(save-excursion
	    (if
		(error-occurred
		    (goto-character 2000)
		    (search-reverse (concat "Rog-O-Matic XIV" " (CMU) "))
		    (search-forward "")
		    (kill-to-end-of-line)
		    (insert-string
			(concat (current-time) " - " (users-login-name))))
		(progn
		    (error-occurred
			(beginning-of-file)
			(search-forward ": version XIII,")
			(end-of-line)
			(forward-character)
			(set-mark)
			(beginning-of-file)
			(wipe-region))
		    (beginning-of-file)
		    (insert-string "/*")
		    (newline)
		    (insert-string 
			(concat " * " (current-buffer-name)
			    ": Rog-O-Matic XIV" " (CMU) "
			    (current-time) " - " (users-login-name)))
		    (newline)
		    (insert-string
			(concat
			    " * Copyright (C) 1985 by A. Appel, "
			    "G. Jacobson, L. Hamey, and M. Mauldin"))
		    (newline)
		    (insert-string " */")
		    (newline))))	
	(setq buffer-is-modified buffer-was-modified)))
