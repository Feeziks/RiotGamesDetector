import os
import shutil
import string
from typing import Dict, List, Iterator, Set
from win32 import win32api, win32security
import ctypes
from ctypes import wintypes
import time
from datetime import datetime

# List of directories to ignore
ignored_directories = [
    r"C:\Windows",
    r"C:\Users\Default",
    r"C:\Users\Public",
    r"C:\ProgramData",
    r"C:\Users\All Users",
    r"C:\$Recycle.Bin",
    r"C:\Program Files\WindowsApps",
    r"C:\Program Files (x86)\Microsoft",
    r"C:\Program Files (x86)\Microsoft SDKs",
    r"C:\Program Files (x86)\Microsoft.NET",
    r"C:\Program Files (x86)\Windows Defender",
    r"C:\Program Files (x86)\Windows Kits",
    r"C:\Program Files (x86)\Windows Media Player",
    r"C:\Program Files\Common Files",
    r"C:\Program Files\dotnet",
    r"C:\Program Files\Reference Assemblies",
    r"C:\Program Files\Windows Defender",
    r"C:\Program Files\Windows Media Player",
    r"C:\$WinREAgent",
    r"C:\Users\Nick\AppData"
]

# Global set to store Riot Games related file paths
riot_games_files: Set[str] = set()

def get_digital_signature_info(file_path: str) -> Dict[str, str]:
    result = {
        "version": "Unknown",
        "subject": "Unknown",
        "issuer": "Unknown",
        "is_signed": False
    }
    try:
        # Get file version info
        info = win32api.GetFileVersionInfo(file_path, "\\")
        ms = info['FileVersionMS']
        ls = info['FileVersionLS']
        result["version"] = f"{win32api.HIWORD(ms)}.{win32api.LOWORD(ms)}.{win32api.HIWORD(ls)}.{win32api.LOWORD(ls)}"
        
        # Check for digital signature
        try:
            sig_info = win32api.GetFileVersionInfo(file_path, '\\VarFileInfo\\Signature')
            result["is_signed"] = True
        except:
            # If there's no signature info, the file is not signed
            pass

        if result["is_signed"]:
            # Try to get signer info
            try:
                signer_info = win32api.GetFileVersionInfo(file_path, '\\StringFileInfo\\040904B0\\CompanyName')
                result["subject"] = signer_info
            except:
                pass

            # Try to get issuer info (this might not always be available)
            try:
                issuer_info = win32api.GetFileVersionInfo(file_path, '\\StringFileInfo\\040904B0\\LegalCopyright')
                result["issuer"] = issuer_info
            except:
                pass

    except Exception as e:
        pass
    return result

def file_generator(directory: str) -> Iterator[str]:
    """
    Generate file paths in the given directory without including subdirectories.
    """
    try:
        with os.scandir(directory) as entries:
            for entry in entries:
                if entry.is_file():
                    yield entry.path
    except Exception as e:
        pass

def process_directory(directory: str):
    """
    Process all .exe files in the given directory, checking their digital signatures.
    Store file paths related to Riot Games.
    """
    global riot_games_files

    # Check if the current directory should be ignored
    if any(directory.lower().startswith(ignored_dir.lower()) for ignored_dir in ignored_directories):
        return

    for file_path in file_generator(directory):
        if file_path.lower().endswith('.exe'):
            try:
                signature_info = get_digital_signature_info(file_path)
                if signature_info['is_signed']:
                    # Check for Riot Games in subject or issuer
                    if ("riot games" in signature_info['subject'].lower() or 
                        "riot games" in signature_info['issuer'].lower()):
                        riot_games_files.add(file_path)
            except Exception as e:
                pass
    
    # Process subdirectories
    try:
        with os.scandir(directory) as entries:
            for entry in entries:
                if entry.is_dir():
                    process_directory(entry.path)
    except Exception as e:
        pass

def get_top_level_directories() -> List[str]:
    top_level_dirs = []
    for drive in string.ascii_uppercase:
        drive_path = f"{drive}:\\"
        if os.path.exists(drive_path):
            top_level_dirs.append(drive_path)
    return top_level_dirs

def remove_riot_games_files(riot_games_files: Set[str]):
    """
    Remove directories containing Riot Games files.
    First, identify all unique parent directories, then remove them.
    """
    # Identify unique parent directories
    unique_parent_dirs = set()
    for file_path in riot_games_files:
        parent_dir = os.path.dirname(file_path)
        unique_parent_dirs.add(parent_dir)

    # Remove the directories
    removed_dirs = set()
    for dir in unique_parent_dirs:
        try:
            shutil.rmtree(dir)
            removed_dirs.add(dir)
        except Exception as e:
            pass

if __name__ == "__main__":
    try:
        while True:
            print(f"\n--- Starting scan at {datetime.now().strftime('%Y-%m-%d %H:%M:%S')} ---")
            
            # Reset global set for each iteration
            riot_games_files.clear()

            directories = get_top_level_directories()
            for directory in directories:
                print(f"Processing drive: {directory}")
                process_directory(directory)
            
            if len(riot_games_files) > 0:
                print(f"\nRiot Games Related Files: {len(riot_games_files)}")
                for file_path in sorted(riot_games_files):
                    print(f"- {file_path}")
                remove_riot_games_files(riot_games_files)
            else:
                print("No Riot Games related files found.")

            print("\nScan completed. Waiting for next scan..")
            
            # Wait for one hour (3600 seconds) before the next iteration
            time.sleep(3600)

    except KeyboardInterrupt:
        print("\nProgram terminated by user.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
    finally:
        print("Program exited.")