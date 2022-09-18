((nil . ((eval . (let ((root (expand-file-name (projectile-project-root))))
                   (setq-local
                    flycheck-gcc-include-path (list (concat root "include"))
                    compile-command (concat "make -C " root " all")))))))
