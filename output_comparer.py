import re

# File paths
output_file = "output.txt"
model_output_file = "expected_outputs/text_files/sparse_1.txt"

def extract_values_from_output(file_path):
    """Extracts cell values from output.txt while handling 'Zero Error' in State."""
    values = {}
    with open(file_path, "r") as f:
        for line in f:
            match = re.match(r"(\w\d+) : .*?--> Value: ([\w\-\.!#]+), State: (\w+)", line)
            if match:
                cell, value, state = match.groups()
                # If the state is "Zero Error", treat it as "#DIV/0!"
                if state == "Zero":
                    value = "#DIV/0!"
                values[cell] = value
    return values

def extract_values_from_model(file_path):
    """Extracts cell values from model_output.txt."""
    values = {}
    with open(file_path, "r") as f:
        for line in f:
            match = re.match(r"(\w\d+): ([\w\-\.!#]+)", line)
            if match:
                cell, value = match.groups()
                values[cell] = value
    return values

# Extract values from both files
output_values = extract_values_from_output(output_file)
model_values = extract_values_from_model(model_output_file)

# Compare values
differences = []
for cell, output_value in output_values.items():
    model_value = model_values.get(cell, "MISSING")  # Default to "MISSING" if not in model_output.txt
    if output_value != model_value:
        if(model_value == "#DIV" and output_value == "#DIV/0!"):
            continue
        differences.append(f"Mismatch at {cell}: output.txt has {output_value}, but model_output.txt has {model_value}")

# Print results
if differences:
    print("\n".join(differences))
else:
    print("✅ All values match!")
