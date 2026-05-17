import pandas as pd
import matplotlib.pyplot as plt

print("1. Loading extracted_1.csv...")
try:
    df = pd.read_csv("extracted_1.csv")
except FileNotFoundError:
    print("Error: extracted_1.csv not found in this folder. Are you in the results directory?")
    exit()

# OMNeT++ CSV-R format usually stores vector data in a 'vecvalue' column
val_col = 'vecvalue' if 'vecvalue' in df.columns else 'value'

if val_col not in df.columns:
    print(f"Error: Could not find the data column. The CSV columns are: {df.columns}")
    exit()

print("2. Extracting delay values...")
all_values = []

# Loop through the rows to extract the numbers
for val in df[val_col].dropna():
    # If scavetool grouped them into space-separated strings
    if isinstance(val, str):
        try:
            values = [float(x) for x in val.split()]
            all_values.extend(values)
        except ValueError:
            pass # Skip any non-numeric header rows that might have slipped in
    else:
        # If it exported them as individual float rows
        all_values.append(float(val))

if not all_values:
    print("Error: No valid numerical data points were found in the column.")
    exit()

print(f"-> Successfully extracted {len(all_values)} data points.")
print("3. Generating the histogram...")

# Create and style the plot
plt.figure(figsize=(10, 6))
plt.hist(all_values, bins=50, color='skyblue', edgecolor='black')

plt.title('Histogram of delaySignal')
plt.xlabel('Delay Time (seconds)')
plt.ylabel('Frequency')
plt.grid(axis='y', alpha=0.75)

# Save directly to an image file
plt.savefig('delay_histogram.png', dpi=300)
print("-> Done! Saved as 'delay_histogram.png' in the current folder.")
