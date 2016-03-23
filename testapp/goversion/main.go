package main

import (
    "gopkg.in/alecthomas/kingpin.v2"
    "github.com/shibukawa/tobubus"
"os"
	"fmt"
	"log"
)

var (
    app = kingpin.New("sample", "tobubus sample plugin/host")
	pluginCommand = app.Command("plugin", "plugin mode")
	pluginPipeName = pluginCommand.Arg("pipe", "Unix domain socket/Windows named pipe name").Required().String()
	hostCommand = app.Command("host", "Host mode")
	hostPipeName = hostCommand.Arg("pipe", "Unix domain socket/Windows named pipe name").Required().String()
)

type Calculator struct {
}

func (c Calculator) Fib(n int64) int64 {
	if n < 2 {
		return n
	}
	return c.Fib(n-2) + c.Fib(n-1)
}

func main() {
	kingpin.CommandLine.HelpFlag.Short('h')
	switch kingpin.MustParse(app.Parse(os.Args[1:])) {
	case pluginCommand.FullCommand():
		Plugin(*pluginPipeName)
	case hostCommand.FullCommand():
		Host(*hostPipeName)
	}
}

func Plugin(pipeName string) {
	plugin, err := tobubus.NewPlugin(pipeName, "github.com/qtpm/QtTobubus/test/sample")
	if err != nil {
		log.Fatalln(err)
	}
	plugin.Publish("/plugin/calculator", &Calculator{})
	err = plugin.ConnectAndServe()
	fmt.Println(err)
}

func Host(pipeName string) {
	host := tobubus.NewHost(pipeName)
	host.Publish("/calculator", &Calculator{})
	err := host.ListenAndServer()
	fmt.Println(err)
}