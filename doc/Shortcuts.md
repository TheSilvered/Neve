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

### Advanced editing

- `[y]s` delete the selection
- `[Y]` delete the current line
- `[^Y]` delete until the end of the current line
- `[r]s` replace selection
- `[R]` replace current line
- `[^R]` replace until the end of the current line
- `[d]` indent current line
- `[D]` dedent current line

- `[g]` global selection of a given pattern
- `[G]` incremental global selection (for each match ask to select or skip)

- `[m]c` start/stop recoring macro `c`
- `[M]c` play macro `c`

- `[t]` repeat last edit

### Advanced movement

- `[b]c` move before the next `c` in the line
- `[B]c` move before the previous `c` in the line
- `[^B]` repeat last `b` or `B` command
- `[a]c` move after the next `c` in the line
- `[A]c` move after the previous `c` in the line
- `[^A]` repeat last `a` or `A` command
- `[q]` move to the matching parenthesis

- `[f]` find a given pattern foreward
- `[F]` find backward
- `[n]` repeat the search foreward
- `[N]` repeat the search backward

### Command variants with `[space]`

- `[space]m` duplicate cursor at the end of the movement
- `[space][z][b]` fuzzy search buffers
- `[space][z][f]` fuzzy search files in the curren working directory
- `[space][z][t]` fuzzy search in the current buffer
- `[space][h]s` find selection foreward
- `[space][H]s` find selection backward

