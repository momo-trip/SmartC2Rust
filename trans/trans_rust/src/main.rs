
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


