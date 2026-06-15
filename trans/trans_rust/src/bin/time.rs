use trans_rust::*;
fn main() {
    let args: Vec<String> = std::env::args().collect();
    let code: i32 = {
    use std::io::Write;

    // set_program_name (argv[0]);
    rust_set_program_name(args.first().map(String::as_str));

    // command_line = getargs (argc, argv);
    let command_line = rust_getargs(&args);

    // run_command (command_line, &res);
    let mut res = RESUSE::default();
    rust_run_command(&command_line, &mut res);

    // summarize (outfp, output_format, command_line, &res);
    // fflush (outfp);
    let fmt = output_format_get().unwrap_or_default();
    {
        let mut guard = outfp.lock().unwrap();
        match guard.as_mut() {
            Some(OutFp::File(file)) => {
                rust_summarize(file, &fmt, &command_line, &res);
                let _ = file.flush();
            }
            _ => {
                let mut err = std::io::stderr();
                rust_summarize(&mut err, &fmt, &command_line, &res);
                let _ = err.flush();
            }
        }
    }

    // The C code inspects res.waitstatus with WIFSTOPPED/WIFSIGNALED/WIFEXITED.
    // RESUSE in the Rust port only retains the decoded exit code that
    // ExitStatus::code() returns (None for signal/stop termination is stored
    // as 0 by rust_run_command), so the stopped/signalled branches collapse
    // into the exit-status branch. The "shouldn't happen" else branch is
    // therefore unreachable here.
    let status: i32 = res.waitstatus;

    std::io::stdout().flush().unwrap();
    status
    };
    std::process::exit(code);
}
