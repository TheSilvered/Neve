# Neve Documentation

> [!note]
> Most of the functionality is not implemented yet, for now this is a list of
> how I intend everything to work

## Notation

`[a]` this is a key, `[A]` is `[shift]+[a]`, `[^A]` is `[ctrl]+[a]`, `[a][b]` is
`[a]` followed by `[b]`.

The `s` suffix is an immediate selection[^1], `m` is a movement command[^2], `c`
is an arbitrary character.

[^1]: An immediate selection is the selection from the current cursor position
to the end of a cursor movement or a global selection.

<!-- TODO: update when all commands are figured out -->
[^2]: A movement command is any of `ijkluoIJKLUO^J^LbBaAfF`.

## Normal mode

### Basic navigation

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

When using `[shift]` they become "more powerful":

- `[I]` move up until a blank line
- `[J]` move to the end of the current word
- `[K]` move down until a blank line
- `[L]` move to the start of the word
- `[U]` move to the start of the file
- `[O]` move to the end of the file

With `[ctrl]` they have yet another different behavior:

- `[^I]` move up half a screen
- `[^K]` move down half a screen
- `[^J]` move to the end of the previous word
- `[^L]` move to the start of the next word
- `[^U]` move the previous edit position
- `[^O]` move the next edit position

### Basic editing

- `[z]` undo
- `[Z]` redo

- `[w]` save (write)
- `[W]` save all

- `[x]s` cut the selection
- `[X]` cut the current line
- `[^X]` cut until the end of the current line
- `[c]s` copy the selection
- `[C]` copy the current line
- `[^C]` copy until the end of the current line
- `[v]` paste after the cursor
- `[V]` paste before the cursor

- `[e]` enter edit mode
- `[E]` move to the end of the line and enter edit mode (= `[o][e]`)
- `[h]` edit in a new line below the cursor
- `[H]` edit in a new line above the cursor

### Advanced movement

- `[b]c` move before the next `c` in the line
- `[B]c` move before the previous `c` in the line
- `[^B]` repeat last `b` or `B` command
- `[a]c` move after the next `c` in the line
- `[A]c` move after the previous `c` in the line
- `[^A]` repeat last `a` or `A` command

- `[f]` find a given pattern foreward, the cursor is placed at the beginning of
  the first match
- `[F]` find a given pattern backward, the cursor is placed at the end of the
  first match
- `[n]` repeat the search foreward
- `[N]` repeat the search backward
- `[space][h]s` find selection foreward
- `[space][H]s` find selection backward

- `[space][z][b]` fuzzy search buffers
- `[space][z][f]` fuzzy search files in the curren working directory
- `[space][z][t]` fuzzy search in the current buffer

- `[space][p]` move to the matching pair (brakets, quotes, ...)

- `[space]m` duplicate cursor at the end of the movement
- `[space]y` remove all cursors keeping only the first one
- `[space]Y` remove all cursors keeping only the last one

### Advanced editing

- `[space][v]c` select clipboard register, if `c` is a space the system
  clipboard is used, the register remains active until it is changed again

- `[y]s` delete the selection
- `[Y]` delete the current line
- `[^Y]` delete until the end of the current line
- `[r]s` replace selection
- `[R]` replace current line
- `[^R]` replace until the end of the current line
- `[d]` indent current line
- `[D]` dedent current line

- `[q]` delete character before cursor (= `[y][j]`)
- `[Q]` delete character after cursor (= `[y][l]`)
- `[space][q]c` replace character before cursor with `c`
- `[space][Q]c` replace character after cursor with `c`

- `[g]` global selection of a given pattern
- `[G]` incremental global selection (for each match ask to select or skip)

- `[m]c` start/stop recoring macro `c`
- `[M]c` play macro `c`
- `[space][m]` play last recorded/played macro

- `[t]` repeat last edit
- `[T]` repeat last command

## Selection mode

To enter selection mode press `[s]`, to enter line selection mode press `[S]`.

Inside selection mode you can move the cursor(s) and a selection will be made
from the starting position to where the cursor moves. Any command that in normal
mode requires an immediate selection will instead operate on the selected text
and quit selection mode.

Additional keybindings:

- `[h]` pause/resume selection to move the cursor freely
- `[H]` move cursor to the other end of the selection
- `[space][p]` surround the selection with parenthesis
- `[space][s]` surround the selection with square brackets
- `[space][c]` surround the selection with curly brackets
- `[space][q]` surround the selection with double quotes
- `[space][t]` surround the selection with single quotes
- `[^Q]`/`[esc]` quit selection mode

### Text objects

In addition to selection via movement, you can also select various text objects:

- `[p]` text inside parenthesis
- `[P]` text inside parenthesis and the parenthesis
- `[s]` text inside square brackets
- `[S]` text inside square brackets and the square brackets
- `[c]` text inside curly brackets
- `[C]` text inside curly brackets and the curly brackets
- `[q]` text inside double quotes
- `[Q]` text inside double quotes and the double quotes brackets
- `[t]` text inside single quotes
- `[T]` text inside single quotes and the single quotes
- `[w]` word the cursor is on
- `[W]` word the cursor is on and the space around it
- `[e]` the extended word (word + punctuation) the cursor is on
- `[E]` the extended word the cursor is on and the space around it
  around it

### Immediate selection

Immediate selection is required after some commands and quits selection mode
after the first command, you can use `[h]` to remain in selection mode and
perform multiple commands and then press `[h]` again to exit and apply the
action.

In immediate selection text object still work.

## Edit mode

In edit mode the key bindings available are the following:

- `[^A]` move to the start of the line
- `[^E]` move to the end of the line
- `[^F]` move cursor foreward
- `[^B]` move cursor backward
- `[^P]` move cursor up
- `[^N]` move cursor down
- `[^H]` delete backward (backspace)
- `[^K]` delete foreward
- `[^W]` delete previous word
- `[^R]` delete next word
- `[^O]` new line above
- `[^J]` new line below
- `[^T]` indent line
- `[^D]` dedent line
- `[^U]` delete to the start of the line
- `[^L]` delete to the end of the line
- `[^S]` suspend edit mode for one command
- `[^V]` enter a digraph
- `[^Q]`/`[esc]` quit edit mode

## Other key bindings

- `[p]` open command palette
