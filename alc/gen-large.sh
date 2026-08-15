
python3 << 'EOF'
# Generate a 4k line .al file

lines = []
lines.append("ret 0\n")
lines.append("\n")

# Add many simple functions - each function is ~5 lines
# 4000/5 = 800 functions
for i in range(800):
    lines.append(f"fn{i}: => i32\n")
    lines.append("    V :: 0\n")
    lines.append("    V + 1 =V\n")
    lines.append("    ret V\n")
    lines.append("\n")

# Write to file
with open("tests/large.al", "w") as f:
    f.writelines(lines)

# Count lines
with open("tests/large.al") as f:
    count = sum(1 for _ in f)

print(f"Created tests/large.al with {count} lines")
EOF
