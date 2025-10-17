#!/usr/bin/env python3
"""
Shared utilities for PlatformIO pre-build scripts.
"""

import os
import sys


def is_build_task(required_tasks):
    """
    Check if the current PlatformIO task requires pre-build script execution.
    
    Args:
        required_tasks: List of PlatformIO tasks that need the calling script
    
    Returns:
        bool: True if script should run, False if it should be skipped
    """
    # Get caller's filename for logging
    try:
        caller_frame = sys._getframe(1)
        script_name = os.path.basename(caller_frame.f_code.co_filename)
    except (AttributeError, ValueError):
        script_name = os.path.basename(sys.argv[0]) if sys.argv else 'unknown_script'
    
    # Check command line arguments for build tasks
    cmd_args = ' '.join(sys.argv).lower()
    
    # Map PlatformIO direct flags to our task names - only skip for truly non-build tasks
    skip_flags = {
        '--clean': 'clean',
        '--erase': 'erase', 
        '--monitor': 'monitor'
    }
    
    # Check for direct PlatformIO flags that should skip pre-build scripts
    for flag, task_name in skip_flags.items():
        if flag in cmd_args:
            if task_name not in required_tasks:
                print(f"Skipping {script_name} for {task_name} task")
                return False
    
    # Check for --target format for non-build targets only
    skip_targets = ['clean', 'erase', 'monitor', 'metrics-only']
    has_skip_target = any(
        f'--target {task}' in cmd_args or f'-t {task}' in cmd_args 
        for task in skip_targets
        if task not in required_tasks
    )
    
    if has_skip_target:
        print(f"Skipping {script_name} for non-build task")
        return False
    
    # Continue execution for all other tasks (including size, metrics, etc. that may need building)
    return True