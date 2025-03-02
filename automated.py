import subprocess
import os
import sys
from concurrent.futures import ThreadPoolExecutor
import time
import shutil

def clean_excel_files():
    """Remove any existing Excel and LibreOffice files"""
    for file in os.listdir("."):
        if file.endswith((".xlsx", ".ods")):
            try:
                os.remove(file)
            except Exception as e:
                print(f"Warning: Could not remove {file}: {e}")

def run_test(test_path):
    """
    Run a single test case and compare outputs.
    Returns (test_number, success, error_message)
    """
    try:
        if not os.path.exists(test_path):
            return os.path.basename(test_path), False, f"Test file {test_path} not found"

        # Get test number from filename
        test_number = os.path.basename(test_path)[2:-4]  # Extract number from tc{n}.txt

        # Clean up any existing Excel files before starting
        clean_excel_files()

        # Copy test file to tc49.txt (required by parser_and_checker.py)
        with open(test_path, 'r') as source, open('tc49.txt', 'w') as dest:
            dest.write(source.read())

        # Generate output.txt using sheet executable
        sheet_process = subprocess.run(
            ["./sheet", "1000", "1000", "-l", "0"],
            input=open(test_path).read(),
            text=True,
            capture_output=True
        )

        if sheet_process.returncode != 0:
            return test_number, False, f"Sheet process failed: {sheet_process.stderr}"

        # Write sheet output to output.txt
        with open("output.txt", "w") as f:
            f.write(sheet_process.stdout)

        # Generate model output
        parser_process = subprocess.run(
            ["python3", "parser_and_checker.py"],
            capture_output=True,
            text=True
        )

        if parser_process.returncode != 0:
            return test_number, False, f"Parser process failed: {parser_process.stderr}"

        # Compare outputs
        compare_process = subprocess.run(
            ["python3", "output_comparer.py"],
            capture_output=True,
            text=True
        )

        success = "✅ All values match!" in compare_process.stdout
        error_message = compare_process.stdout if not success else "All values match"

        # Clean up Excel files after test completion
        clean_excel_files()

        # Clean up temporary tc49.txt
        if os.path.exists('tc49.txt'):
            os.remove('tc49.txt')

        return test_number, success, error_message

    except Exception as e:
        return os.path.basename(test_path), False, f"Unexpected error: {str(e)}"

def main():
    # Check if testcases directory exists
    if not os.path.exists("testcases"):
        print("Error: 'testcases' directory not found")
        sys.exit(1)

    # Check if sheet executable exists
    if not os.path.exists("./sheet"):
        print("Error: 'sheet' executable not found in current directory")
        sys.exit(1)

    # Initial cleanup of any leftover files
    clean_excel_files()
    if os.path.exists('tc49.txt'):
        os.remove('tc49.txt')

    # Get list of all test files from testcases directory
    test_files = []
    for file in os.listdir("testcases"):
        if file.startswith("tc") and file.endswith(".txt") and file != "tc49.txt":
            test_files.append(os.path.join("testcases", file))

    # Sort test files by test number
    test_files.sort(key=lambda x: int(os.path.basename(x)[2:-4]))
    test_files = test_files[:1]
    if not test_files:
        print("No test files found in testcases directory (format: tc{n}.txt)")
        sys.exit(1)

    print(f"Found {len(test_files)} test files")
    start_time = time.time()

    # Run tests in parallel using ThreadPoolExecutor
    success_count = 0
    failure_count = 0
    with ThreadPoolExecutor(max_workers=1) as executor:  # Single worker to prevent Excel file conflicts
        futures = [executor.submit(run_test, test_path) for test_path in test_files]

        for future in futures:
            test_num, success, message = future.result()

            if success:
                success_count += 1
                print(f"\n✅ Test {test_num} passed")
            else:
                failure_count += 1
                print(f"\n❌ Test {test_num} failed")
                print(f"Error: {message}")

    end_time = time.time()
    duration = end_time - start_time

    # Print summary
    print("\n" + "="*50)
    print("Test Summary:")
    print(f"Total tests: {len(test_files)}")
    print(f"Passed: {success_count}")
    print(f"Failed: {failure_count}")
    print(f"Time taken: {duration:.2f} seconds")
    print("="*50)

    # Final cleanup
    clean_excel_files()
    if os.path.exists('tc49.txt'):
        os.remove('tc49.txt')

    # Return non-zero exit code if any tests failed
    sys.exit(1 if failure_count > 0 else 0)

if __name__ == "__main__":
    main()