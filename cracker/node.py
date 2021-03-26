class Node:
    def __init__(self, value: int):
        self.value = value
        self.prev_node = None
        self.next_node = []
        self._is_explored = False

    def __repr__(self):
        return str(self.value)

    def explored(self, is_explored=None) -> bool:
        if is_explored is not None:
            self._is_explored = is_explored
        return self._is_explored

    def connect_next(self, node) -> None:
        found = False
        for i in range(len(self.next_node)):
            if node == self.next_node[i]:
                self.next_node[i] = node
                found = True
        if not found:
            self.next_node.append(node)
        node.prev_node = self

    def next_paths(self) -> list:
        return self.next_node

    def prev(self):
        return self.prev_node

    def chain(self) -> list:
        values = []
        values.append(self.value)
        nn = self
        while nn.prev() is not None:
            nn = nn.prev()
            values.append(nn.value)
        return values


def next_unexplored_paths(node: Node) -> list:
    possible_paths = node.next_paths()
    valid_paths = []
    for el in possible_paths:
        if not el.explored():
            valid_paths.append(el)
    return valid_paths


def set_explored(node: Node) -> None:
    node.explored(True)
    while node.prev() is not None:
        node = node.prev()
        if not next_unexplored_paths(node):
            node.explored(True)
        else:
            break


def depth(node: Node) -> int:
    return len(node.chain())


def history(node: Node) -> list:
    return node.chain()[:-1]
