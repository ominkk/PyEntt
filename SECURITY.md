# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 0.1.x   | :white_check_mark: |

## Reporting a Vulnerability

If you discover a security vulnerability in PyEnTT, please report it responsibly.

### How to Report

Please send an email to [your-security-email@example.com] with the following information:

1. A description of the vulnerability
2. Steps to reproduce the vulnerability
3. Potential impact of the vulnerability
4. Any suggested fixes or mitigations (if available)

### What to Expect

- We will acknowledge receipt of your report within 48 hours
- We will provide a detailed response within 7 days
- We will work with you to understand and validate the issue
- We will coordinate a disclosure timeline with you

### Disclosure Process

Once we verify a vulnerability:

1. We will fix the vulnerability as soon as possible
2. We will release a new version with the fix
3. We will publish a security advisory
4. We will credit you for the discovery (if desired)

### Security Best Practices

Users of PyEnTT should follow these security best practices:

- Keep dependencies up to date
- Use virtual environments
- Review code before deployment
- Follow principle of least privilege
- Monitor security advisories

### Dependency Security

PyEnTT depends on the following main libraries:

- [EnTT](https://github.com/skypjack/entt) - C++ ECS library
- [nanobind](https://github.com/wjakob/nanobind) - Python/C++ bindings

We actively monitor these projects for security updates and will incorporate fixes promptly.

### Secure Development

We follow these secure development practices:

- Code review for all changes
- Automated testing on multiple platforms
- Static analysis where applicable
- Dependency scanning
- Regular dependency updates

### Contact Information

For security-related questions that don't require vulnerability reporting, please open an issue with the `[security]` tag.

### Acknowledgments

We thank all security researchers who help keep PyEnTT safe and secure.

---

**Note**: Do not open a public issue for security vulnerabilities. Please report them privately as described above.
