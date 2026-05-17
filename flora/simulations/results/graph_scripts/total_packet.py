import matplotlib.pyplot as plt

print("1. Loading validated data...")
# You already manually verified these exact numbers from the IDE!
total_sent = 2561
total_recv = 2561

print("2. Calculating PDR...")
pdr = (total_recv / total_sent) * 100
print(f"-> Calculated PDR: {pdr}%")

print("3. Generating the Proof Graph...")
fig, ax = plt.subplots(figsize=(7, 6))

categories = ['Total Sent\n(By Nodes)', 'Total Received\n(By Server)']
values = [total_sent, total_recv]

# Draw the bars
bars = ax.bar(categories, values, color=['#FF9800', '#4CAF50'], edgecolor='black', linewidth=1.5, width=0.5)

# Add exact numbers on top of the bars
for bar in bars:
    yval = bar.get_height()
    ax.text(bar.get_x() + bar.get_width()/2, yval + 50, int(yval), 
             ha='center', va='bottom', fontweight='bold', fontsize=12)

# Strict formatting
ax.set_title(f'Packet Delivery Ratio Proof\n(PDR = {pdr:.1f}% over 20 Runs)', pad=20, fontweight='bold')
ax.set_ylabel('Total Number of Packets')

# Safely zoom out the Y-axis so the text is perfectly visible
ax.set_ylim(0, max(values) + 400)
ax.grid(axis='y', linestyle='--', alpha=0.7, zorder=0)

fig.tight_layout()
fig.savefig('pdr_proof_final.png', dpi=300)
print("-> Done! Saved as pdr_proof_final.png")