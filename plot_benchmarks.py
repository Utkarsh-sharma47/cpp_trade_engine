import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

try:
    df = pd.read_csv('benchmark_results.csv')
except FileNotFoundError:
    print("Error: benchmark_results.csv not found.")
    exit()

# 1. Setup the visual style (Clean Light Theme for maximum visibility)
plt.style.use('default')
sns.set_theme(style="whitegrid") 
plt.figure(figsize=(12, 7))

# 2. Plot your Engine's Data
plt.plot(df['Orders'], df['Avg_ns'], marker='o', linewidth=2, label='Your Engine - Average', color='#4C72B0')
plt.plot(df['Orders'], df['P50_ns'], marker='s', linestyle='--', label='Your Engine - P50 (Median)', color='#55A868')
plt.plot(df['Orders'], df['P99_ns'], marker='X', linewidth=2, label='Your Engine - P99 (Tail Latency)', color='#C44E52')

# 3. Overlay Industry Benchmarks (Adjusted colors for high contrast on white)
plt.axhline(y=780, color='#D55E00', linestyle=':', linewidth=2, label='NanoARB (Rust HFT) - 780ns')
plt.axhline(y=1000, color='#0072B2', linestyle=':', linewidth=2, label='LMAX Disruptor - 1,000ns')
plt.axhline(y=5000, color='#E69F00', linestyle=':', linewidth=2, label='Standard Crypto Backend - 5,000ns+')

# 4. Formatting the Axes (Bold, Black text for readability)
plt.xscale('log') 
plt.xlabel('Number of Orders Processed (Log Scale)', fontsize=12, color='black', fontweight='bold')
plt.ylabel('Latency per Order (Nanoseconds)', fontsize=12, color='black', fontweight='bold')
plt.title('O(1) Matching Engine Latency vs. Order Volume\n(Virtual Machine Hardware)', fontsize=16, fontweight='bold', color='black')

# Force tick numbers to be distinctly black
plt.tick_params(axis='both', colors='black', labelsize=10)

# Set Y-axis limit
# Cap the Y-axis at 6000ns to zoom in on the critical HFT thresholds
plt.ylim(0, 8000) 

# 5. Create a clean legend with a solid white background frame so lines don't bleed through it
legend = plt.legend(loc='upper left', fontsize=10, frameon=True)
legend.get_frame().set_facecolor('white')
legend.get_frame().set_edgecolor('black')

plt.tight_layout()

# 6. Save the image with a forced white background
plt.savefig('latency_benchmark_graph.png', dpi=300, facecolor='white', bbox_inches='tight')
print("Success! Graph saved as 'latency_benchmark_graph.png'")