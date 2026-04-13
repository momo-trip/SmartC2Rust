# SmartC2Rust

Iterative, Feedback-Driven C-to-Rust Translation via Large Language Models for Safety and Equivalence

## Start with Docker

We prepared the pre-built Docker image, which contains the complete environment (Ubuntu 22.04, Python, Rust, Clang, and all dependencies) ready to run.

### Pull the image

```bash
docker pull ghcr.io/momo-trip/smartc2rust:v1.0
```

### Run

```bash
docker run --rm -it smartc2rust:v1.0
```

This drops you into the container with all tools and source code pre-installed at `/root`.

### Reproduce experiments

```bash
docker run --rm smartc2rust:v1.0 python3 run.py --reproduce-all
```

### LLM API Key

TBA

## Paper

Paper: [arXiv:2409.10506](https://arxiv.org/abs/2409.10506) (ICSE 2026)
🆕 This work has been accepted at [ICSE 2026](https://conf.researchr.org/details/icse-2026/icse-2026-research-track/217/SmartC2Rust-Iterative-Feedback-Driven-C-to-Rust-Translation-via-Large-Language-Mode).


## License

This project is licensed under the [Creative Commons Attribution 4.0 International License](https://creativecommons.org/licenses/by/4.0/).


## Contact
Momoko Shiraishi\
University email: <u>shiraishi@os.is.s.u-tokyo.ac.jp</u>\
(Personal email: <u>momoko.shiraishi36@gmail.com</u>)








