The interpreter is also a recursive solution. An AST lends itself to that, with terminals being given dedicated functions
and non-terminals being given large multi-function functions. This allows the tree structure to be used efficiently. A
large problem during dev was the values for AST_TAG and TOK_TAG not always lining up,
this was solved by functions that converted between the two more structurally, thus preventing errors.