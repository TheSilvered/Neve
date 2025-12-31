# Short-term goals

- Add tests for previous _ctxReplace bugs
- Add tests for new indentation functions
- Add line selection
  - ctxGetCursorSel respects kind of selection
- Add undo history (save _ctxReplace changes)
  - improve _ctxReplace by replacing the smallest different continuous substring
- Add multiple cursors mode to Ctx (ctxCurMove instead duplicates)
- Improve control sequence reading
- Improve key events & harmonies
- Basic search/replace
- Jump history
- Figure out a way of managing multiple buffers (+ better representation?)
- Add clipboard support

# Long-term goals

- Syntax highlighting (both treesitter and in-app)
- Regex search & replace (with PCRE?)
- LSP server implementation
- More correct UNICODE support (text width, right-to-left, hangeul, probably more)
- File explorer/tree
- Options
