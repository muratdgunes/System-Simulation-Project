import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

print("1. Loading scalar data...")
df = pd.read_csv("exttracted_2_totalpacket.csv")

if 'value' not in df.columns:
    print("Error: Could not find 'value' column.")
    exit()

raw_values = df['value'].dropna().values

print("2. Calculating exact 95% boundaries...")
# We strictly calculate the 95% bounds and true min/max
stats = [{
    'med': np.median(raw_values),
    'q1': np.percentile(raw_values, 2.5),   # Bottom of the box (2.5%)
    'q3': np.percentile(raw_values, 97.5),  # Top of the box (97.5%)
    'whislo': np.min(raw_values),           # Bottom whisker (True Min)
    'whishi': np.max(raw_values),           # Top whisker (True Max)
    'label': 'Network Server'               # X-axis label
}]

print("3. Generating the custom 95% Box Plot...")
# The "By The Book" Object-Oriented Approach
fig, ax = plt.subplots(figsize=(6, 6))

# Call bxp directly on the Axes object (ax) instead of the pyplot module (plt)
ax.bxp(stats, 
       patch_artist=True,
       showfliers=False, 
       boxprops=dict(facecolor='#4C92C3', color='blue', linewidth=1.5),
       medianprops=dict(color='#D32F2F', linewidth=2.5), 
       whiskerprops=dict(color='blue', linewidth=1.5),
       capprops=dict(color='blue', linewidth=1.5))

# Formatting using the Axes object
ax.set_title('Total Received Packets\n(Box represents 95% statistical probability)')
ax.set_ylabel('Packets Received')
ax.grid(axis='y', linestyle='--', alpha=0.7)

plt.tight_layout()
plt.savefig('packets_95_boxplot2.png', dpi=300)
print("-> Done! Saved as packets_95_boxplot.png")