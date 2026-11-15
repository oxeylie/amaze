{ pkgs, ... }:
let
  mkShellConfig =
    {
      add_nvimrc ? false,
      ...
    }:
    {
      buildInputs = with pkgs; [
        vulkan-headers
        ncurses
      ];
      packages =
        with pkgs;
        [
          clang-tools
          shader-slang
          vulkan-loader
          vulkan-validation-layers
        ]
        ++ pkgs.lib.optionals add_nvimrc [
          compiledb
        ];
      shellHook =
        let
          ts = pkgs.vimPlugins.nvim-treesitter.withPlugins (p: [ p.slang ]);
          slang_parser = "${pkgs.vimPlugins.nvim-treesitter.builtGrammars.slang}/parser";
          nvim-lsps = [
            {
              name = "slangd";
              package = pkgs.shader-slang;
              luaOptions = ''
                {
                  cmd = { 'slangd' },
                  filetypes = { 'shaderslang' },
                  capabilities = vim.lsp.protocol.make_client_capabilities(),  
                  root_dir = function(bufnr, on_dir)
                    local fname = vim.api.nvim_buf_get_name(bufnr)
                    on_dir(vim.fs.dirname(vim.fs.find('slangdconfig.json', { path = fname, upward = true })[1])
                      or vim.fs.dirname(fname))
                  end,
                  on_init = function(client)
                    local path = client.root_dir .. '/slangdconfig.json'
                    local f = io.open(path, 'r')
                    if not f then return end
                    local ok, cfg = pcall(vim.json.decode, f:read('*a'))
                    f:close()
                    if not ok then return end

                    client.settings = cfg

                    client:notify('workspace/didChangeConfiguration', { settings = client.settings })
                  end,
                }
              '';
            }
          ];

          nvim-treesitter = ''
            local parser, ts = vim.env.SLANG_PARSER, vim.env.SLANG_TS
            if not (parser and ts) then return end

            vim.opt.runtimepath:append(ts .. '/runtime')
            vim.treesitter.language.add('slang', { path = parser })
            vim.treesitter.language.register('slang', 'shaderslang')

            vim.api.nvim_create_autocmd('FileType', {
              pattern = 'shaderslang',
              callback = function() pcall(vim.treesitter.start) end,
            })
          '';

          nvim-lsp-configs = pkgs.lib.concatStringsSep "\n" (
            map (lsp: ''
              vim.lsp.config('${lsp.name}', ${lsp.luaOptions})
              vim.lsp.enable('${lsp.name}')
            '') nvim-lsps
          );

          nvim-header-fn = ''
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
          '';

          nvim-rc = pkgs.lib.concatStringsSep "\n" [
            nvim-treesitter
            nvim-lsp-configs
            nvim-header-fn
          ];
        in
        pkgs.lib.optionalString add_nvimrc ''
          export SHELL_RC="$PWD/.zsh-shell"
          export NVIM_RC="$PWD/.nvim.lua"
          export SLANG_TS=${ts}
          export SLANG_PARSER=${slang_parser}

          cat > $NVIM_RC <<-'EOF'
          ${nvim-rc}
          EOF

          cat > $SHELL_RC <<-'EOF'
          TRAPEXIT() {
            rm "$SHELL_RC"
            rm "$NVIM_RC"
          }
          EOF

          if command -v >/dev/null zsh; then
            exec zsh --rcs -i -c "source .zsh-shell; zsh -i"
          fi
        '';
    };
in
rec {
  minimal = pkgs.mkShell (mkShellConfig { });
  full = pkgs.mkShell (mkShellConfig {
    add_nvimrc = true;
  });
  default = minimal;
}
