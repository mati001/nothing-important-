import os
import shutil
import subprocess

def create_file(path, content):
    with open(path, 'w') as f:
        f.write(content)

def setup_test_environment():
    shutil.rmtree('test_src', ignore_errors=True)
    shutil.rmtree('test_dst', ignore_errors=True)

    os.makedirs('test_src/subdir')
    os.makedirs('test_src/other')

    create_file('test_src/file.txt', 'Hello')
    create_file('test_src/subdir/subfile.txt', 'SubHello')
    create_file('test_src/other/otherfile.txt', 'OtherHello')

    # Create symlink in the same folder
    os.symlink('file.txt', 'test_src/link_same')

    # Create symlink to subdirectory file
    os.symlink('subdir/subfile.txt', 'test_src/link_to_sub')

    # Create symlink to parent directory file
    os.symlink('../file.txt', 'test_src/subdir/link_to_parent')

    # Create symlink with absolute path
    abs_path = os.path.abspath('test_src/other/otherfile.txt')
    os.symlink(abs_path, 'test_src/link_abs')

def run_backup():
    result = subprocess.run(['./backup', 'test_src', 'test_dst'], capture_output=True, text=True)
    if result.returncode != 0:
        print("❌ Backup failed:")
        print(result.stderr)
        exit(1)
    else:
        print("✅ Backup ran successfully.")

def verify_symlinks():
    print("\n🔍 Verifying symlinks in test_dst:\n")

    def check_link(path, expected_target):
        if not os.path.islink(path):
            print(f"❌ {path} is not a symlink!")
            return
        target = os.readlink(path)
        status = "✅" if target == expected_target else "❌"
        print(f"{status} {path} -> {target} (expected: {expected_target})")

    check_link('test_dst/link_same', 'file.txt')
    check_link('test_dst/link_to_sub', 'subdir/subfile.txt')
    check_link('test_dst/subdir/link_to_parent', '../file.txt')
    abs_expected = os.path.abspath('test_dst/other/otherfile.txt')  # ✅ not test_src
    check_link('test_dst/link_abs', abs_expected)
if __name__ == "__main__":
    setup_test_environment()
    run_backup()
    verify_symlinks()
