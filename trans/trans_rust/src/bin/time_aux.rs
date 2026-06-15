use trans_rust::*;
fn main() {
    let args: Vec<String> = std::env::args().collect();
    let code: i32 = {
    use std::io::Write;

    let mut rc: i32 = 0;

    // Manual emulation of getopt(argc, argv, "e:m:b:s:H:k:h").
    // The options 'e', 'm', 'b', 's', 'H' and 'k' each take an argument;
    // 'h' takes none.
    const ARG_OPTS: [char; 6] = ['e', 'm', 'b', 's', 'H', 'k'];

    let mut i = 1usize; // skip argv[0]
    while let Some(arg) = args.get(i) {
        // getopt stops at "--" and at the first non-option argument.
        if arg == "--" || !arg.starts_with('-') || arg == "-" {
            break;
        }

        let chars: Vec<char> = arg[1..].chars().collect();
        let mut ci = 0usize;
        while let Some(&c) = chars.get(ci) {
            // Resolve the option's argument (if any), mirroring getopt's
            // behaviour of accepting either "-oVALUE" or "-o VALUE".
            let optarg: Option<String> = if ARG_OPTS.contains(&c) {
                let rest: String = chars[ci + 1..].iter().collect();
                if !rest.is_empty() {
                    ci = chars.len();
                    Some(rest)
                } else {
                    i += 1;
                    ci += 1;
                    match args.get(i) {
                        Some(next) => Some(next.clone()),
                        // Missing required argument.
                        None => rust_errx(EXIT_CANCELED, "invalid option"),
                    }
                }
            } else {
                ci += 1;
                None
            };

            match c {
                // Set exit code.
                'e' => rc = rust_get_exit_code(&optarg.unwrap_or_default()),
                // Malloc.
                'm' => rust_do_malloc(&optarg.unwrap_or_default()),
                // Help.
                'h' => rust_usage_time_aux(),
                // Busy loop, waste CPU cycles in user mode.
                'b' => rust_do_busy_user_sleep(&optarg.unwrap_or_default()),
                // Sleep without wasting CPU cycles in user mode.
                's' => rust_do_sleep(&optarg.unwrap_or_default()),
                // Half busy loop, half sleep.
                'H' => rust_do_half_busy_sleep(&optarg.unwrap_or_default()),
                // Busy loop, wasting kernel/system time.
                'k' => rust_do_busy_sys_sleep(&optarg.unwrap_or_default()),
                // errx (EXIT_CANCELED, "invalid option");
                _ => rust_errx(EXIT_CANCELED, "invalid option"),
            }
        }

        i += 1;
    }

    std::io::stdout().flush().unwrap();
    rc
    };
    std::process::exit(code);
}
