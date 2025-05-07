## Framework Botnet
Framework for creating and managing a botnet.

# Basic information
 1. Various attack methods (automated messaging, phishing, social engineering).
 2. Wide range of distribution: email, SMS, messengers (Telegram, WhatsApp), social networks (Facebook, Instagram), etc.
 3. Software distribution: sending and auto-installation of software on recipients' devices.
 4. Botnet management: centralized orchestration of infected devices.
 5. Stealth technologies: polymorphism, antivirus bypass, encryption, obfuscation.
 6. Data collection: stealing passwords, payment details, stored browser data.
 7. Multi-platform: support for Windows, Linux, MacOS, Android, iOS.
 8. Captcha bypass: built-in captcha solving methods (CaptchaSolver).
 9. Self-protection: hiding traces of presence, protection from analysis.
 10. Monitoring and reports: collection of statistics, generation of reports on work done.

# 2. Getting Started
This section describes the general procedure for building and starting the project.
Required components
CMake, compilers (GCC/Clang) - if the project includes building C++ code via CMake.
Libraries (Boost, OpenSSL, etc.) - depends on the specific code.
Docker - if containerization is planned.
Building the project
The build script is located in ./scripts/build.sh:
./scripts/build.sh
The output results in an executable file in the build/ folder.
Running the project
./build/your_project_executable
# 3. basic commands
Abbreviated instructions for working with the project (scripts in the scripts/ folder):
Build:
./scripts/build.sh
Tests:
./scripts/test.sh
Load testing:
./scripts/load_test.sh
Deploy:
./scripts/deploy.sh
# 4. Settings and configuration
4.1 System configuration
The config/system.conf file:
[system].
log_level=INFO
max_threads=10
4.2 Configuration of bots
File config/bots.conf:
[bot1]
task_type=DOWNLOAD
priority=HIGH
params=url:http://localhost/file.txt,destination:/tmp/file.txt
# 5. Running and configuring with Docker
Build the image:
docker build -t image -f docker/Dockerfile .

Launching the container:
docker run -d -p 8080:8080 image

# 6. Additional information
The following is a more generalized description of the project, touching on mass mailings, phishing, data collection, etc.

6.1 Mass mailings
Email, SMS, Telegram, WhatsApp, Facebook, Instagram.
Templates for personalization and increased efficiency.
6.2 Phishing and social engineering
Sending phishing links leading to fake websites.
URL masking (shorteners, redirects).
6.3 Software distribution
Attaching files.
Automatic download/installation.
6.4 Botnet management
Centralized commands for infected devices (BotNetManager).
Automatic bot updates.
6.5 Self-protection and stealthy distribution
Antivirus bypass, polymorphism.
Protection from analysis: encryption, obfuscation.
6.6 Data collection
Collection of credentials, payment information.
Send to C2.
6.7 Support for different operating systems
Using specific vulnerabilities for Windows/Linux/MacOS/Android/iOS.
6.8 Bypass captchas and other protections
Machine learning methods or recognition services.
6.9 Self-protection features
Automatic trace removal, encryption, obfuscation.
6.10 Monitoring and reporting
Statistics on emails opened, links clicked.
Generation of summary reports.

## This is a tutorial demonstrating the possibility, the responsibility for use lies with you!
