{ pkgs, ... }:
{
  default = pkgs.mkShell {
    packages =
      with pkgs;
      [
        gnumake
        ncurses
      ]
      ++ (if pkgs.stdenv.isLinux then [ gcc ] else [ clang ]);

    shellHook = ''
      export SHELL_RC="$PWD/.zsh-shell"
      export NVIM_RC="$PWD/.nvim.lua"

      cat > $SHELL_RC <<-'EOF'
      TRAPEXIT() {
        rm "$NVIM_RC"
      }
      EOF

      cat > $NVIM_RC <<-'EOF'
      local function insert_header()
        local first = vim.api.nvim_buf_get_lines(0, 0, 1, false)[1] or ""
        if first:find("^/%*%*%-") then
          vim.notify("Header already present", vim.log.levels.WARN)
          return
        end

        local name = vim.fn.expand("%:t")   -- file name without directory
        vim.api.nvim_buf_set_lines(0, 0, 0, false, {
          "/**-----------------------------------------------------------------------*",
          " * " .. name,
          " * ",
          " * Copyright (C) " .. os.date("%Y") .. " @oxeylie",
          " * SPDX-License-Identifier: AGPL-3.0-or-later",
          " *-----------------------------------------------------------------------**/",
          "",
        })
      end

      vim.api.nvim_create_user_command("Header", insert_header, {})
      vim.keymap.set("n", "<leader>h", insert_header, { desc = "Insert file header" })

      vim.api.nvim_create_autocmd("BufNewFile", {
        pattern = { "*.c", "*.h" },
        callback = insert_header,
      })
      EOF

      if command -v >/dev/null zsh; then
        exec zsh --rcs -i -c "source .zsh-shell; zsh -i"
      fi
    '';
  };
}
