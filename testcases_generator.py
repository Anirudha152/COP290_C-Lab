import random
import string
from collections import defaultdict

class DependencyGraph:
    def __init__(self):
        self.graph = defaultdict(set)
        
    def add_edge(self, from_cell, to_cell):
        self.graph[from_cell].add(to_cell)
        
    def has_cycle(self, cell):
        visited = set()
        def dfs(current):
            visited.add(current)
            for neighbor in self.graph[current]:
                if neighbor in visited or dfs(neighbor):
                    return True
            visited.remove(current)
            return False
        return dfs(cell)
    
    def add_dependency(self, from_cell, to_cells):
        # Temporarily add the edge
        for to_cell in to_cells:
            self.add_edge(from_cell, to_cell)
            if self.has_cycle(from_cell):
                # If cycle is detected, remove the edge and return False
                self.graph[from_cell].remove(to_cell)
                return False
        return True

def get_random_cell():
    col = random.choice(string.ascii_uppercase[:5])  # A to E
    row = random.randint(1, 10)
    return f"{col}{row}"

def generate_constant_assignment(dependencies):
    cell = get_random_cell()
    value = random.randint(-100, 100)
    return f"{cell}={value}", cell, True

def generate_reference_assignment(dependencies):
    cell = get_random_cell()
    ref_cell = get_random_cell()
    
    # Check if adding this reference would create a cycle
    if dependencies.add_dependency(ref_cell, {cell}):
        return f"{cell}={ref_cell}", cell, True
    return None, None, False

def generate_arithmetic_operation(dependencies):
    operations = ['+', '-', '*', '/']
    cell = get_random_cell()
    ref_cell1 = get_random_cell()
    
    # Either use two references or one reference and one constant
    if random.choice([True, False]):
        ref_cell2 = get_random_cell()
        # Check for cycles with both references
        if dependencies.add_dependency(ref_cell1, {cell}) and dependencies.add_dependency(ref_cell2, {cell}):
            operation = random.choice(operations)
            return f"{cell}={ref_cell1}{operation}{ref_cell2}", cell, True
    else:
        # Check for cycles with one reference
        if dependencies.add_dependency(ref_cell1, {cell}):
            operation = random.choice(operations)
            constant = random.randint(1, 100)  # Avoid 0 for division
            return f"{cell}={ref_cell1}{operation}{constant}", cell, True
    return None, None, False

def get_cells_in_range(top_left, bottom_right):
    col1, row1 = top_left[0], int(top_left[1:])
    col2, row2 = bottom_right[0], int(bottom_right[1:])
    
    cells = set()
    for col in string.ascii_uppercase[string.ascii_uppercase.index(col1):string.ascii_uppercase.index(col2)+1]:
        for row in range(row1, row2+1):
            cells.add(f"{col}{row}")
    return cells

def generate_range_function(dependencies):
    functions = ['SUM', 'AVG', 'STDEV', 'MIN', 'MAX']
    cell = get_random_cell()
    
    # Generate range corners
    col1 = random.choice(string.ascii_uppercase[:3])  # A to C
    row1 = random.randint(1, 8)
    col2 = random.choice(string.ascii_uppercase[string.ascii_uppercase.index(col1):5])  # col1 to E
    row2 = random.randint(row1, 10)
    
    range_cells = get_cells_in_range(f"{col1}{row1}", f"{col2}{row2}")
    
    # Check if the range would create any cycles
    all_valid = True
    for range_cell in range_cells:
        if not dependencies.add_dependency(range_cell, {cell}):
            all_valid = False
            break
    
    if all_valid:
        function = random.choice(functions)
        return f"{cell}={function}({col1}{row1}:{col2}{row2})", cell, True
    return None, None, False

def generate_sleep_function(dependencies):
    cell = get_random_cell()
    sleep_time = random.randint(1, 5)
    return f"{cell}=SLEEP({sleep_time})", cell, True

def generate_query():
    return get_random_cell()

def generate_testcase(min_edits=3, max_edits=10, min_queries=2, max_queries=5):
    n = random.randint(min_edits, max_edits)  # Number of edits
    m = random.randint(min_queries, max_queries)  # Number of queries
    
    dependencies = DependencyGraph()
    edits = []
    cells_used = set()
    
    while len(edits) < n:
        operation_type = random.choice([
            'constant',
            'reference',
            'arithmetic',
            'range',
            'sleep'
        ])
        
        edit = None
        cell = None
        valid = False
        
        if operation_type == 'constant':
            edit, cell, valid = generate_constant_assignment(dependencies)
        elif operation_type == 'reference':
            edit, cell, valid = generate_reference_assignment(dependencies)
        elif operation_type == 'arithmetic':
            edit, cell, valid = generate_arithmetic_operation(dependencies)
        elif operation_type == 'range':
            edit, cell, valid = generate_range_function(dependencies)
        else:  # sleep
            edit, cell, valid = generate_sleep_function(dependencies)
        
        if valid and cell not in cells_used:
            edits.append(edit)
            cells_used.add(cell)
    
    queries = [generate_query() for _ in range(m)]
    
    return n, m, edits, queries

def generate_testfile(num_testcases=5, filename="tc1.txt"):
    with open(filename, 'w') as f:
        # Write number of testcases
        f.write(f"{num_testcases}\n")
        
        # Generate each testcase
        for _ in range(num_testcases):
            n, m, edits, queries = generate_testcase()
            
            # Write number of edits and queries
            f.write(f"{n} {m}\n")
            
            # Write edits
            for edit in edits:
                f.write(f"{edit}\n")
            
            # Write queries
            for query in queries:
                f.write(f"{query}\n")

# Generate test cases
if __name__ == "__main__":
    generate_testfile(num_testcases=1, filename="tc1.txt")