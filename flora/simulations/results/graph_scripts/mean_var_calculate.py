import pandas as pd
import numpy as np

df = pd.read_csv("delay_signal_EMERGENCY_CASE.csv")
val_col = 'vecvalue' if 'vecvalue' in df.columns else 'value'

all_delays = []
for val in df[val_col].dropna():
    if isinstance(val, str):
        all_delays.extend([float(x) for x in val.split()])
    else:
        all_delays.append(float(val))

mean_delay = np.mean(all_delays)
std_delay = np.std(all_delays)
ci95_delay = 1.96 * (std_delay / np.sqrt(len(all_delays)))

print(f"Delay Mean: {mean_delay:.4f} seconds")
print(f"Delay Std Dev: {std_delay:.4f}")
print(f"Delay 95% CI: +/- {ci95_delay:.4f}")
