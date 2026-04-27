
// Rust port of yank
use std::fs::{File, OpenOptions};
use std::io::{Read, Write};
use std::os::fd::AsRawFd;
use std::process::{Command, Stdio};
    };
    let buf = rust_read_input();
    let mut fields = rust_parse_fields(&buf, &cfg.delim);
    rust_fix_line_origins(&buf, &mut fields);

    let sel = rust_tmain(&cfg, &buf, &fields);
    let code = if let Some(f) = sel {
        rust_yank_selection(&cfg, &buf, &f);
        0
    } else {
        1
    };
    std::process::exit(code);
}



    rust_buffer_trim_left(&mut buf.borrow_mut());
    rust_equal("Hello\n\n ", &buffer_string_as_str(&buf.borrow()));

    let buf = rust_buffer_new_with_copy(sample).expect("buffer_new_with_copy failed");
    rust_buffer_trim_right(&mut buf.borrow_mut());
    rust_equal("  Hello", &buffer_string_as_str(&buf.borrow()));
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    use std::io::Write;

    let result = run_selected_test(&args);

    std::io::stdout().flush().unwrap();
    std::process::exit(result);
}


    rust_equal("Hello\n\n ", &buffer_string_as_str(&buf.borrow()));

    let buf = rust_buffer_new_with_copy(sample).expect("buffer_new_with_copy failed");
    rust_buffer_trim_right(&mut buf.borrow_mut());
    rust_equal("  Hello", &buffer_string_as_str(&buf.borrow()));
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    use std::io::Write;

    let result = run_selected_test(&args);

    std::io::stdout().flush().unwrap();
    std::process::exit(result);
}


    rust_equal("Hello\n\n ", &buffer_string_as_str(&buf.borrow()));

    let buf = rust_buffer_new_with_copy(sample).expect("buffer_new_with_copy failed");
    rust_buffer_trim_right(&mut buf.borrow_mut());
    rust_equal("  Hello", &buffer_string_as_str(&buf.borrow()));
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    use std::io::Write;

    let result = run_selected_test(&args);

    std::io::stdout().flush().unwrap();
    std::process::exit(result);
}

