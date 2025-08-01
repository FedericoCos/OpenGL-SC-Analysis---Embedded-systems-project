import pandas as pd
import matplotlib.pyplot as plt
import argparse
import sys

def plot_statistics(csv_path: str, save_plots: bool = False):
    try:
        # Read the CSV data into a pandas DataFrame
        df = pd.read_csv(csv_path)
        print(f"Successfully loaded '{csv_path}' with {len(df)} frames of data.")
    except FileNotFoundError:
        print(f"Error: The file '{csv_path}' was not found.", file=sys.stderr)
        sys.exit(1)
    except pd.errors.EmptyDataError:
        print(f"Error: The file '{csv_path}' is empty.", file=sys.stderr)
        sys.exit(1)
    
    df = df[:10000]

    frame_axis = df.index

    fig1, ax1 = plt.subplots(figsize=(12, 6))
    ax1.plot(frame_axis, df['FPS'], label='FPS', color='green')
    ax1.set_title('Frames Per Second (FPS) Over Time')
    ax1.set_xlabel('Frame Number')
    ax1.set_ylabel('FPS')
    ax1.legend()
    ax1.grid(True, linestyle='--', alpha=0.6)

    if save_plots:
        plt.savefig('plot_fps.png', dpi=300, bbox_inches='tight')

    fig2, ax2 = plt.subplots(figsize=(12, 6))
    ax2.plot(frame_axis, df['FrameTime(ms)'], label='Total Frame Time (ms)', color='red')
    ax2.plot(frame_axis, df['AvgFrame(ms)'], label='Avg Frame Time (ms)', color='orange', linestyle='--')
    ax2.plot(frame_axis, df['CPUTime(ms)'], label='CPU Render Time (ms)', color='blue', alpha=0.8)
    ax2.plot(frame_axis, df['GPUWait(ms)'], label='GPU Wait Time (ms)', color='purple', alpha=0.8)
    ax2.set_title('Frame Timing Analysis')
    ax2.set_xlabel('Frame Number')
    ax2.set_ylabel('Time (ms)')
    ax2.legend()
    ax2.grid(True, linestyle='--', alpha=0.6)

    if save_plots:
        plt.savefig('plot_timings.png', dpi=300, bbox_inches='tight')
        
    fig3, ax3 = plt.subplots(figsize=(12, 6))
    ax3_twin = ax3.twinx() # Create a second y-axis

    ax3.plot(frame_axis, df['DrawCalls'], color='tab:blue', label='Draw Calls')
    ax3_twin.plot(frame_axis, df['Triangles'], color='tab:red', label='Triangles')

    ax3.set_title('Draw Calls and Triangles per Frame')
    ax3.set_xlabel('Frame Number')
    ax3.set_ylabel('Draw Calls', color='tab:blue')
    ax3_twin.set_ylabel('Triangles', color='tab:red')
    
    # Combine legends from both axes
    lines, labels = ax3.get_legend_handles_labels()
    lines2, labels2 = ax3_twin.get_legend_handles_labels()
    ax3_twin.legend(lines + lines2, labels + labels2, loc='upper left')
    ax3.grid(True, linestyle='--', alpha=0.3)

    if save_plots:
        plt.savefig('plot_calls_tris.png', dpi=300, bbox_inches='tight')

    # --- Plot 4: Memory Usage ---
    fig4, ax4 = plt.subplots(figsize=(12, 6))
    ax4_twin = ax4.twinx()

    ax4.plot(frame_axis, df['VRAM(MB)'], color='tab:cyan', label='Total VRAM (MB)')
    ax4_twin.plot(frame_axis, df['Upload(KB)'], color='tab:pink', label='Upload per Frame (KB)', alpha=0.7)

    ax4.set_title('Memory Usage')
    ax4.set_xlabel('Frame Number')
    ax4.set_ylabel('VRAM Allocated (MB)', color='tab:cyan')
    ax4_twin.set_ylabel('Data Uploaded (KB)', color='tab:pink')
    
    lines, labels = ax4.get_legend_handles_labels()
    lines2, labels2 = ax4_twin.get_legend_handles_labels()
    ax4_twin.legend(lines + lines2, labels + labels2, loc='upper left')
    ax4.grid(True, linestyle='--', alpha=0.3)

    if save_plots:
        plt.savefig('plot_memory.png', dpi=300, bbox_inches='tight')

    # --- Show all plots ---
    if not save_plots:
        plt.tight_layout()
        plt.show()
    else:
        print("All plots saved as PNG files in the current directory.")


if __name__ == '__main__':
    # Set up command-line argument parsing
    parser = argparse.ArgumentParser(
        description="A script to parse and plot performance stats from a CSV file.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        'csv_path',
        type=str,
        help='The path to the input stats.csv file.'
    )
    parser.add_argument(
        '--save',
        action='store_true',
        help='Save the plots as PNG files instead of displaying them interactively.'
    )
    args = parser.parse_args()

    # Run the main function
    plot_statistics(args.csv_path, args.save)