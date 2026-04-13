import subprocess
from pathlib import Path
import os

def debug_with_pexpect(c_run_script, target_dir, breakpoints=None, use_breakpoints=True):
    """
    pexpectを使ってrust-gdbを対話的に制御（タイムアウトなし）
    
    Args:
        c_run_script: 実行するプログラムのパス
        target_dir: ターゲットディレクトリ
        breakpoints: ブレークポイントのリスト
        use_breakpoints: ブレークポイントを使用するかどうか（True/False）
    """
    try:
        import pexpect
    except ImportError:
        print("Error: pexpect not installed. Install with: pip install pexpect")
        return False
    
    c_run_script = Path(c_run_script)
    
    if not c_run_script.exists():
        print(f"Error: Executable not found at {c_run_script}")
        return False
    
    if breakpoints is None:
        breakpoints = ["main"]
    
    print(f"\n=== Debugging {c_run_script} with pexpect-controlled rust-gdb ===")
    print(f"Use breakpoints: {use_breakpoints}\n")
    
    # rust-gdbを起動
    gdb = pexpect.spawn(f"rust-gdb -q {c_run_script}")
    gdb.logfile = open("/tmp/gdb_log.txt", "wb")
    
    try:
        # GDBプロンプトを待つ（タイムアウトなし）
        gdb.expect(r"\(gdb\)")
        
        # 設定
        gdb.sendline("set print pretty on")
        gdb.expect(r"\(gdb\)")
        
        # ブレークポイントを設定（use_breakpointsがTrueの場合のみ）
        if use_breakpoints:
            for bp in breakpoints:
                print(f"Setting breakpoint at {bp}")
                gdb.sendline(f"break {bp}")
                gdb.expect(r"\(gdb\)")
                print(gdb.before.decode())
        
        # プログラムを実行
        print("\nRunning program...")
        gdb.sendline("run")
        gdb.expect(r"\(gdb\)")
        print(gdb.before.decode())
        
        # バックトレースを表示
        print("\nBacktrace:")
        gdb.sendline("backtrace")
        gdb.expect(r"\(gdb\)")
        print(gdb.before.decode())
        
        if use_breakpoints:
            # ブレークポイント使用時のみ実行
            
            # ローカル変数を表示
            print("\nLocal variables:")
            gdb.sendline("info locals")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # 次の行へ
            print("\nNext line:")
            gdb.sendline("next")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # 変数を表示
            print("\nPrinting variables:")
            gdb.sendline("info locals")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # 続行
            print("\nContinuing...")
            gdb.sendline("continue")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # 次のブレークポイントでも変数確認
            print("\nAt next breakpoint - checking Rust variables:")
            gdb.sendline("info args")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            gdb.sendline("info locals")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # 最後まで実行（すべてのブレークポイントをスキップ）
            print("\nRunning to completion...")
            gdb.sendline("continue")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
        
        # 終了
        print("\nQuitting GDB...")
        gdb.sendline("quit")
        gdb.expect(pexpect.EOF)
        
        print("\n=== Debug session completed ===")
        
        return True
        
    except Exception as e:
        print(f"\nError during debugging: {e}")
        import traceback
        traceback.print_exc()
        return False
    finally:
        gdb.close()
        print("Full log saved to /tmp/gdb_log.txt")

