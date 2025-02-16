# git tips


-   To list untracked files
    ```bash
    git ls-files --others --exclude-standard
    ```
    If you need to pipe the output to xargs, it is wise to mind white spaces using `git ls-files -z` and `xargs -0`:
    ```bash
    git ls-files -z -o --exclude-standard | xargs -0 git add
    ```
    Nice alias for adding untracked files:
    ```bash
    au = !git add $(git ls-files -o --exclude-standard)
    ```

-   Delete untracked files:
    ```sh
    git clean -dfx
    ```

