(defun
    (fcopy fname
	(beginning-of-file)
	(set-mark)
	(search-forward "version X")
	(wipe-end-of-line)
	(insert-string "III, 11/01/83 */")
	(beginning-of-line)
	(delete-to-killbuffer)
	(insert-file "copy")
	(sit-for 5)
	(write-current-file)
	(delete-window)
	(pop-to-buffer "flist")
	(next-line)
	(beginning-of-line)
	(set-mark)
	(end-of-line)
	(setq fname (region-to-string))
	(delete-other-windows)
	(visit-file fname)
	(beginning-of-file)
    )
)