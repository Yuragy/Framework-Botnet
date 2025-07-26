
# Botnet Framework  
Framework for creating and managing a botnet

![](./scr/IoT_M2M_communication.png)

---

## 1&nbsp;· Functionality
1. Various attack methods – automated messaging, phishing, social-engineering.  
2. Wide distribution channels – e-mail, SMS, messengers Telegram, WhatsApp, social networks Facebook, Instagram.  
3. Software distribution – send and auto install software on recipients devices.  
4. Botnet management – centralized orchestration of infected devices.  
5. Stealth technologies – polymorphism, AV bypass, encryption, obfuscation.  
6. Data collection – steal passwords, payment details, browser data.  
7. Multi platform – Windows · Linux · macOS · Android · iOS.  
8. Captcha bypass – built in solver CaptchaSolver.  
9. Self protection – hide traces, defend against analysis.  
10. Monitoring & reports – collect statistics, generate work done reports.  

---

## 2&nbsp;· Getting Started

> Prerequisites  
> - CMake + C/C++ compiler GCC/Clang – when building C++ via CMake  
> - Libraries Boost, OpenSSL – depending on project modules  
> - Docker – if containerization is required  

### Build
The build script lives in ./scripts/build.sh:

```bash
./scripts/build.sh
executable appears in ./build/
````

### Run

```bash
./build/your_project_executable
```

---

## 3 · Basic Commands

| Action           | Script                   |
| ---------------- | ------------------------ |
| **Build**        | ./scripts/build.sh     |
| **Tests**        | ./scripts/test.sh      |
| **Load testing** | ./scripts/load_test.sh |
| **Deploy**       | ./scripts/deploy.sh    |

---

## 4 · Settings & Configuration

config/system.conf

```
log_level   = INFO
max_threads = 10
```

config/bots.conf

```
task_type = DOWNLOAD
priority  = HIGH
params    = url:http://localhost/file.txt,destination:/tmp/file.txt
```

---

## 5 · Running with Docker

Build image:
```
docker build -t image -f docker/Dockerfile .
```

Launch container:
```
docker run -d -p 8080:8080 image
```

---

## 6 · Additional Information

| Topic                      | Highlights                                                                    |
| -------------------------- | ----------------------------------------------------------------------------- |
| **Mass mailings**          | E-mail, SMS, Telegram, WhatsApp, Facebook, Instagram · Personalised templates |
| **Phishing / SE**          | Phishing URLs → fake sites · URL masking shorteners, redirects              |
| **Software distribution**  | File attachments · Auto-download & install                                    |
| **Botnet management**      | Central commands via BotNetManager · Auto updates                       |
| **Stealthy distribution**  | AV bypass, polymorphism, encryption, obfuscation                              |
| **Data collection**        | Credentials, payment info → C2                                                |
| **OS support**             | Platform-specific exploits Windows / Linux / macOS / Android / iOS            |
| **Captcha bypass**         | ML recognition or external services                                           |
| **Self-protection**        | Auto-trace removal, encryption, obfuscation                                   |
| **Monitoring & reporting** | Mail open-rate, link clicks, summary reports                                  |

---

## Useful Resources

1. **Master of Puppets: Analyzing And Attacking A Botnet For Fun And Profit** – Cutwail/Pushdo architecture & C2 weaknesses.
   [https://arxiv.org/abs/1511.06090](https://arxiv.org/abs/1511.06090)
2. **Peer-to-Peer Botnets** – survey of flat P2P botnet designs and detection.
   [https://www.cs.ucf.edu/\~czou/research/P2PBotnets-bookChapter.pdf](https://www.cs.ucf.edu/~czou/research/P2PBotnets-bookChapter.pdf)
3. **Fast Flux 101** – how cyber-criminals rotate DNS to harden infrastructure.
   [https://unit42.paloaltonetworks.com/fast-flux-101/](https://unit42.paloaltonetworks.com/fast-flux-101/)
4. **Inside the Infamous Mirai IoT Botnet: A Retrospective Analysis** – infection vectors & DDoS anatomy.
   [https://blog.cloudflare.com/inside-mirai-the-infamous-iot-botnet-a-retrospective-analysis/](https://blog.cloudflare.com/inside-mirai-the-infamous-iot-botnet-a-retrospective-analysis/)
5. **D-LNBot: A Scalable, Cost-Free and Covert Hybrid Botnet on Bitcoin’s Lightning Network** – embedding commands in LN payments.
   [https://arxiv.org/abs/2112.07623](https://arxiv.org/abs/2112.07623)

---

## Disclaimer

This repository is provided for educational purposes only and intended for authorized security research.
Use of these materials in unauthorized or illegal activities is strictly prohibited.
