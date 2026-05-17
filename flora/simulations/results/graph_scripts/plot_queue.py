import pandas as pd
import matplotlib.pyplot as plt
import ast

print("1. Loading vector data...")
try:
    df = pd.read_csv("queue_data.csv")
except FileNotFoundError:
    print("Error: queue_data.csv not found.")
    exit()

def get_column(df, target_names):
    for col in df.columns:
        if str(col).strip().lower() in [name.lower() for name in target_names]:
            return col
    return None

time_col = get_column(df, ['vectime', 'time', 'Vectime', 'Time'])
val_col = get_column(df, ['vecvalue', 'value', 'Vecvalue', 'Value'])
name_col = get_column(df, ['name', 'Name'])

print("2. Parsing timeline arrays...")
fig, ax = plt.subplots(figsize=(10, 4))
plotted_anything = False

for index, row in df.iterrows():
    name = str(row[name_col]) if name_col else f"Queue {index}"
    raw_times = row[time_col]
    raw_values = row[val_col]
    
    try:
        if isinstance(raw_times, str) and '[' in raw_times:
            times = ast.literal_eval(raw_times)
            values = ast.literal_eval(raw_values)
        elif isinstance(raw_times, str):
            times = [float(x) for x in raw_times.split()]
            values = [float(x) for x in raw_values.split()]
        else:
            times = [float(raw_times)]
            values = [float(raw_values)]
            
        times_in_hours = [t / 3600 for t in times]
        
        color = '#D32F2F' if 'high' in name.lower() else '#1976D2'
        label_name = 'High Priority Queue' if 'high' in name.lower() else 'Low Priority Queue'
        
        ax.step(times_in_hours, values, where='post', color=color, linewidth=1.5, label=label_name, alpha=0.8)
        plotted_anything = True
        
    except Exception as e:
        pass

print("3. Generating the graph...")
ax.set_title('Gateway Queue Length Over Time (Replica #0)', fontweight='bold', pad=15)
ax.set_xlabel('Simulation Time (Hours)')
ax.set_ylabel('Packets in Queue')

ax.set_yticks(range(0, 3)) 
ax.set_ylim(-0.1, 1.5)
ax.grid(axis='both', linestyle='--', alpha=0.5)

# --- THE FIX: Deduplicate the legend labels ---
handles, labels = ax.get_legend_handles_labels()
by_label = dict(zip(labels, handles)) # Dictionaries naturally overwrite duplicates!
ax.legend(by_label.values(), by_label.keys(), loc='upper right')
# ----------------------------------------------

fig.tight_layout()
fig.savefig('queue_timeline_final.png', dpi=300)
print("-> Done! Saved as queue_timeline_final.png")