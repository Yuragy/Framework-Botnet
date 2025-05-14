                                 Framework for creating and managing a botnet 

![](./scr/IoT_M2M_communication.png)

# 1. Functionality
 1. Various attack methods (automated messaging, phishing, social engineering).
 2. Wide range of distribution: email, SMS, messengers (Telegram, WhatsApp), social networks (Facebook, Instagram).
 3. Software distribution: sending and auto-installation of software on recipients' devices.
 4. Botnet management: centralized orchestration of infected devices.
 5. Stealth technologies: polymorphism, antivirus bypass, encryption, obfuscation.
 6. Data collection: stealing passwords, payment details, stored browser data.
 7. Multi-platform: support for Windows, Linux, MacOS, Android, iOS.
 8. Captcha bypass: built-in captcha solving methods (CaptchaSolver).
 9. Self-protection: hiding traces of presence, protection from analysis.
 10. Monitoring and reports: collection of statistics, generation of reports on work done.

# 2. Getting Started
- This section describes the general procedure for building and starting the project.
- Required components
CMake, compilers (GCC/Clang) - if the project includes building C++ code via CMake.
- Libraries (Boost, OpenSSL, etc.) - depends on the specific code.
Docker - if containerization is planned.
- Building the project
The build script is located in ./scripts/build.sh:
./scripts/build.sh
The output results in an executable file in the build/ folder.
- Running the project
./build/your_project_executable
# 3. basic commands
Abbreviated instructions for working with the project (scripts in the scripts/ folder):
1. Build:
./scripts/build.sh
2. Tests:
./scripts/test.sh
3. Load testing:
./scripts/load_test.sh
4. Deploy:
./scripts/deploy.sh
# 4. Settings and configuration
-  System configuration
The config/system.conf file:
[system].
log_level=INFO
max_threads=10
-  Configuration of bots
File config/bots.conf:
[bot1]
task_type=DOWNLOAD
priority=HIGH
params=url:http://localhost/file.txt,destination:/tmp/file.txt

# 5. Running and configuring with Docker
 - Build the image:   docker build -t image -f docker/Dockerfile .

 - Launching the container:  docker run -d -p 8080:8080 image

# 6. Additional information

The following is a more generalized description of the project, touching on mass mailings, phishing, data collection.

1. Mass mailings
Email, SMS, Telegram, WhatsApp, Facebook, Instagram.
Templates for personalization and increased efficiency.
2. Phishing and social engineering
Sending phishing links leading to fake websites.
URL masking (shorteners, redirects).
3. Software distribution
Attaching files.
Automatic download/installation.
4. Botnet management
Centralized commands for infected devices (BotNetManager).
Automatic bot updates.
5. Self-protection and stealthy distribution
Antivirus bypass, polymorphism.
Protection from analysis: encryption, obfuscation.
6. Data collection
Collection of credentials, payment information.
Send to C2.
7. Support for different operating systems
Using specific vulnerabilities for Windows/Linux/MacOS/Android/iOS.
8. Bypass captchas and other protections
Machine learning methods or recognition services.
9. Self-protection features
Automatic trace removal, encryption, obfuscation.
10. Monitoring and reporting
Statistics on emails opened, links clicked.
Generation of summary reports.


## Useful resources on this topic:

1. **Master of Puppets: Analyzing And Attacking A Botnet For Fun And Profit** : 
   A deep dive into the centralized architecture of the Cutwail/Pushdo spam botnet: C\&C server layouts, command-exchange protocols, management software vulnerabilities, and red-team counter-attack techniques.
   [https://arxiv.org/abs/1511.06090](https://arxiv.org/abs/1511.06090)

2. **Peer-to-Peer Botnets** : 
   A survey of P2P botnets: how nodes in a flat network distribute commands without a single point of failure, detection methods, and examples like Trojan.
   [https://www.cs.ucf.edu/\~czou/research/P2PBotnets-bookChapter.pdf](https://www.cs.ucf.edu/~czou/research/P2PBotnets-bookChapter.pdf)

3. **Fast Flux 101: How Cybercriminals Improve the Resilience of Their Infrastructure** : 
   A hands-on guide to fast-flux techniques: dynamically rotating DNS records to hide C\&C servers behind a proxy network, making takedown and blocking efforts much harder.
   [https://unit42.paloaltonetworks.com/fast-flux-101/](https://unit42.paloaltonetworks.com/fast-flux-101/)

4. **Inside the Infamous Mirai IoT Botnet: A Retrospective Analysis** : 
   A retrospective look at the Mirai IoT botnet: infection vectors for “smart” devices, the design of its distributed network, DDoS-attack characteristics, and lessons for securing IoT ecosystems.
   [https://blog.cloudflare.com/inside-mirai-the-infamous-iot-botnet-a-retrospective-analysis/](https://blog.cloudflare.com/inside-mirai-the-infamous-iot-botnet-a-retrospective-analysis/)

5. **D-LNBot: A Scalable, Cost-Free and Covert Hybrid Botnet on Bitcoin’s Lightning Network** : 
   Describes a hybrid botnet architecture where commands are embedded in Lightning Network payments: combining C\&C servers with a distributed proxy layer for high anonymity and resilience.
   [https://arxiv.org/abs/2112.07623](https://arxiv.org/abs/2112.07623)


## This is a tutorial demonstrating the possibility, the responsibility for use lies with you!
