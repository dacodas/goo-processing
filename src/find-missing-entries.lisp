(with-open-file (entry-numbers-file "/home/dacoda/projects/goo-processing/results/entry-numbers")
  (let* ((file-length (file-length entry-numbers-file))
         (file-contents (make-array file-length :element-type 'character)))

    (read-sequence file-contents entry-numbers-file :end file-length)

    (let ((current-number 0)
          (missing-count 0))
      (loop for line in (cl-ppcre:split "\\s+" file-contents)
         do (let ((entry-number (parse-integer line)))
              (if (not (eq entry-number current-number))
                  (loop while (< current-number entry-number)
                     do (progn
                          (format t "~A~%" current-number)
                          (incf missing-count)
                          (incf current-number))))
              (incf current-number)))
      missing-count)))
