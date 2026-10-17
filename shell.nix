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
        local name = vim.fn.expand("%:~:.")
        local header = {
          "/**-----------------------------------------------------------------------*",
          " * " .. name,
          " * ",
          " * Copyright (C) " .. os.date("%Y") .. " @oxeylie",
          " * SPDX-License-Identifier: AGPL-3.0-or-later",
          " *-----------------------------------------------------------------------**/",
        }

        local first = vim.api.nvim_buf_get_lines(0, 0, 1, false)[1] or ""
        if first:find("^/%*%*%-") then
          -- find the end of the existing header (search the first 20 lines)
          local lines = vim.api.nvim_buf_get_lines(0, 0, 20, false)
          local last
          for i, l in ipairs(lines) do
            if l:find("%*%-+%*%*/%s*$") then last = i break end
          end
          if last then
            vim.api.nvim_buf_set_lines(0, 0, last, false, header)
            return
          end
          -- no terminator found: fall through and insert a fresh one
        end

        table.insert(header, "")
        vim.api.nvim_buf_set_lines(0, 0, 0, false, header)
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
