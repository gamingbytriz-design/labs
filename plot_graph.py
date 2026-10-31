import subprocess
import matplotlib.pyplot as plt
import re

N = 100
player_balances = []

print(f"Running {N} simulations...")

for seed in range(N):
    process = subprocess.run(
        ['casino.exe', str(seed), 'auto'],
        capture_output=True,
        text=True
    )
    
    match = re.search(r'FINAL_BALANCE: (\d+)', process.stdout)
    if match:
        player_balances.append(int(match.group(1)))
    else:
        player_balances.append(0)

plt.figure(figsize=(10, 6))
plt.plot(range(N), player_balances, label='Player Balance', color='blue')
plt.xlabel('Seed')
plt.ylabel('Balance (cents)')
plt.title('Player Balance vs Seed')
plt.grid(True)
plt.show()
