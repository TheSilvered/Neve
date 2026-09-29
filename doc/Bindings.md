# Neve Documentation

> [!note]
> Most of the functionality is not implemented yet, for now this is a list of
> how I intend everything to work

## Notation

`[a]` this is a key, `[A]` is `[shift]+[a]`, `[^A]` is `[ctrl]+[a]`, `[a][b]` is
`[a]` followed by `[b]`.

The `s` suffix is an immediate selection, `c` is an arbitrary character.

## Normal mode

### Movement

Imagine `i`, `j`, `k`, `l` as arrow keys and `u` and `o` as the `[home]` and
`[end]` keys:

```
↖    ↑    ↗
 [u][i][o]
←[j][k][l]→
```

- `[i]` move up one line
- `[j]` move left one character
- `[k]` move down one line
- `[l]` move right one character
- `[u]` move to the start of the current line
- `[o]` move to the end of the current line
- `[I]` move up until a blank line
- `[J]` move to the start of the word
- `[K]` move down until a blank line
- `[L]` move to the start of the next word
- `[U]` move to the start of the file
- `[O]` move to the end of the file
- `[^I]` move up half a screen
- `[^K]` move down half a screen
- `[^J]` move to the end of the previous word
- `[^L]` move to the end of the word
- `[^U]` move the previous edit position
- `[^O]` move the next edit position
- `[b]c` move before the next `c` in the line
- `[B]c` move before the previous `c` in the line
- `[^B]` repeat last `b` or `B` command
- `[a]c` move after the next `c` in the line
- `[A]c` move after the previous `c` in the line
- `[^A]` repeat last `a` or `A` command
- `[f]` find a given pattern foreward and move to it
- `[F]` find a given pattern backward and move to it
- `[n]` search the last pattern given to `[f]` or `[F]` foreward
- `[N]` search the last pattern given to `[f]` or `[F]` backward
- `[space][f]s` find selection foreward (can be repeated with `[n]` or `[N]`)
- `[space][F]s` find selection backward (can be repeated with `[n]` or `[N]`)
- `[space][z][b]` fuzzy search buffers
- `[space][z][f]` fuzzy search files in the curren working directory
- `[space][z][t]` fuzzy search in the current buffer
- `[space][p]` move to the matching pair (brakets, quotes, ...)

### Basic editing

- `[z]` undo
- `[Z]` redo
- `[^Z]` open undo history
- `[y]s` delete the selection
- `[Y]` delete the current line
- `[^Y]` delete until the end of the current line
- `[q]` delete the character before the cursor (= `[y][j]`)
- `[Q]` delete the character after the cursor (= `[y][l]`)
- `[^Q][j]c` change the character before the cursor
- `[^Q][l]c` change the character after the cursor
- `[space][d]` indent
- `[space][D]` dedent
- `[space][s]` save
- `[space][S]` save all
- `[space][b]` insert a new line at the cursor position ("break" the line)
- `[space][j]` join the current line with the one below, remove the indentation
  and add a space

### Clipboard

- `[x]s` cut the selection
- `[X]` cut the current line
- `[^X]` cut until the end of the current line
- `[c]s` copy the selection
- `[C]` copy the current line
- `[^C]` copy until the end of the current line
- `[v]` paste before the cursor
- `[V]` paste after the cursor
- `[^V]c` change the clipboard register

### Cursor duplication

- `[d][i]` duplicate to the line above
- `[d][k]` duplicate to the line below
- `[d][J]` duplicate to the previous word start
- `[d][L]` duplicate to the next word start
- `[d][^J]` duplicate to the previous word end
- `[d][^K]` duplicate to the next word end
- `[d][u]` duplicate to the start of the line
- `[d][o]` duplicate to the end of the line

- `[d][b]c` duplicate to before the next `c` in the line
- `[d][B]c` duplicate to before the previous `c` in the line
- `[d][a]c` duplicate to after the next `c` in the line
- `[d][A]c` duplicate to after the previous `c` in the line

- `[d][f]` duplicate to the resulting position of `[f]`
- `[d][F]` duplicate to the resulting position of `[F]`
- `[d][n]` duplicate to the resulting position of `[n]`
- `[d][N]` duplicate to the resulting position of `[N]`
- `[d][space][f]s` duplicate to selection foreward
- `[d][space][F]s` duplicate to selection backward
- `[d][space][p]` duplicate to the matching pair

- `[D]` repeat last reduplication command

- `[^D][u]` delete all cursors except the first
- `[^D][o]` delete all cursors except the last
- `[^D][^D]` delete all cursors except the initial one

### Macros

- `[m]c` start/stop recoring macro `c`
- `[M]c` play macro `c`
- `[space][m]` play last recorded/played macro

### GUI management

- `[w][j]` focus the panel on the left
- `[w][l]` focus the panel on the right
- `[w][i]` focus the panel above
- `[w][k]` focus the panel below

<!-- TODO: resizing and switching places (dedicated mode?) -->

### Switch mode

- `[p]` open command palette
- `[e]` enter edit mode
- `[E]` move to the end of the line and enter edit mode (= `[o][e]`)
- `[^E]` move to the start of the line and enter edit mode (= `[u][e]`)
- `[h]` edit in a new line below the cursor
- `[H]` edit in a new line above the cursor
- `[s]s` enter selection mode, starting with the immediate selection
- `[S]` enter line selection mode
- `[^S]` enter search and replace mode
- `[^D]` enter cursor reduplication mode
- `[r]s` replace selection (= `[y]s[e]`)
- `[R]` replace current line (= `[Y][e]` )
- `[^R]` replace until the end of the current line (= `[^Y][e]`)

## Selection mode

The movement is the same as in normal mode, but a selection starts from each
cursor's initial position.

Additional commands:

- `[h]` pause/resume selection to move the cursor freely
- `[H]` move cursor to the other end of the selection


### Switch mode

All the following commands will go back to normal mode after applying their
effects unless otherwise specified.

- `[c]` copy the selection
- `[x]` cut the selection
- `[v]` replace the selection(s) with the content of the clipboard
- `[r]` remove the selection and enter edit mode
- `[d]` indent the selection
- `[D]` dedent the selection
- `[s][p]` / `[s][(]` / `[s][)]` surround the selection with parenthesis
- `[s][b]` / `[s][[]` / `[s][]]` surround the selection with square brackets
- `[s][c]` / `[s][{]` / `[s][}]` surround the selection with curly brackets
- `[s][n]` / `[s][<]` / `[s][>]` surround the selection with angle brackets
- `[s][d]` / `[s]["]` surround the selection with double quotes
- `[s][s]` / `[s][']` surround the selection with single quotes
- `[s][t]` prompt for a tag name and surround the selection with `<name>` and
  `</name>`
- `[s][w]c` surround the selection with character `c`
- `[s][W]` prompt for a series of characters and surround the selection with it
- `[^C]`/`[^Q]`/`[esc]` quit selection mode

### Immediate selection

The immediate selection can be a normal movement that will select the text
between the initial position of the cursor and the resulting position after the
movement. In addition it can be one of the following text objects:

- `[p]` / `[(][i]` / `[)][i]` text inside parenthesis
- `[P]` / `[(][a]` / `[)][a]` text inside parenthesis and the parenthesis
- `[b]` / `[[][i]` / `[]][i]` text inside square brackets
- `[B]` / `[[][a]` / `[[][a]` text inside square brackets and the square
  brackets
- `[c]` / `[{][i]` / `[}][i]` text inside curly brackets
- `[C]` / `[{][a]` / `[}][a]` text inside curly brackets and the curly brackets
- `[n]` / `[<][i]` / `[>][i]` text inside angle brackets
- `[N]` / `[<][a]` / `[>][a]` text inside angle brackets and the curly brackets
- `[d]` / `["][i]` text inside double quotes
- `[D]` / `["][a]` text inside double quotes and the double quotes brackets
- `[s]` / `['][i]` text inside single quotes
- `[S]` / `['][a]` text inside single quotes and the single quotes
- `[w]` word the cursor is on
- `[W]` word the cursor is on and the space around it
- `[e]` the extended word (word + punctuation) the cursor is on
- `[E]` the extended word the cursor is on and the space around it
- `[t]` the text inside a pair of tags
- `[T]` the text inside a pair of tags and the tags themselves

There is also one additional command:

- `[h]` hold the immediate selection and allow for more movement commands
  until `[h]` is pressed again

## Edit mode

In edit mode the key bindings available are the following:

- `[^A]` move to the start of the line
- `[^E]` move to the end of the line
- `[^F]` move cursor foreward
- `[^B]` move cursor backward
- `[^K]` move cursor to previous word
- `[^L]` move cursor to next word
- `[^P]` move cursor up (previous line)
- `[^N]` move cursor down (next line)
- `[^Z]` delete backward (backspace)
- `[^X]` delete foreward
- `[^W]` delete previous word
- `[^R]` delete next word
- `[^O]` new line above (over)
- `[^U]` new line below (under)
- `[^D]` indent line
- `[^S]` dedent line
- `[^T]` delete to the start of the line
- `[^Y]` delete to the end of the line
- `[^H]` (TODO: reconsider) suspend edit mode for one command
- `[^V]` enter a digraph
- `c` insert the character
- `[^C]`/`[^Q]`/`[esc]` quit edit mode
