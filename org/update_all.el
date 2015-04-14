;;; update_all.el -- Brief introduction here.

;; Author: YangYingchao <yangyingchao@gmail.com>

;;; Commentary:

;;; Code:


(defun update_all_posts ()
  "Update all posts."
  (interactive)
  (mapc   (lambda (x)
            (interactive)
            (find-file x)
            (yc/prepare-post t)
            )
          (directory-files "/mnt/data/Work/yangyingchao.github.io/org" nil "org$"))



  )
(provide 'update_all)

;; Local Variables:
;; coding: utf-8
;; indent-tabs-mode: nil
;; End:

;;; update_all.el ends here
