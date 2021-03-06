package main

import (
	"fmt"
	"github.com/shibukawa/tobubus"
	"gopkg.in/alecthomas/kingpin.v2"
	"log"
	"os"
)

var (
	app            = kingpin.New("sample", "tobubus sample plugin/host")
	pluginCommand  = app.Command("plugin", "plugin mode")
	pluginPipeName = pluginCommand.Arg("pipe", "Unix domain socket/Windows named pipe name").Required().String()
	hostCommand    = app.Command("host", "Host mode")
	hostPipeName   = hostCommand.Arg("pipe", "Unix domain socket/Windows named pipe name").Required().String()
)

type Calculator struct {
	host   *tobubus.Host
	plugin *tobubus.Plugin
}

func (c Calculator) Fib(n int64) int64 {
	log.Println("Fib: ", n)
	if n < 2 {
		return n
	}
	return c.Fib(n-2) + c.Fib(n-1)
}

func (c Calculator) FibF(n float64) float64 {
	log.Println("Fib: ", n)
	if n < 2 {
		return n
	}
	return c.FibF(n-2) + c.FibF(n-1)
}

func (c Calculator) Callback(path, method string, params []interface{}) {
	log.Println("Callback: ", path, method, params)
	if c.host != nil {
		result, err := c.host.Call(path, method, params)
		log.Println("Callback Result:", result, err)
	} else {
		result, err := c.plugin.Call(path, method, params)
		log.Println("Callback Result:", result, err)
	}
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
	plugin, err := tobubus.NewPlugin(pipeName, "github.com/qtpm/QtTobubus/samples/goversion")
	if err != nil {
		log.Fatalln(err)
	}
    plugin.PreventCrush = false
	log.Println("publish to /plugin/calculator")
	plugin.Publish("/plugin/calculator", &Calculator{
		plugin: plugin,
	})
	log.Println("connect to " + pipeName)
	err = plugin.ConnectAndServe()
	fmt.Println(err)
}

func Host(pipeName string) {
	host := tobubus.NewHost(pipeName)
    host.PreventCrush = false
	log.Println("publish to /calculator")
	host.Publish("/calculator", &Calculator{
		host: host,
	})
	log.Println("connect to " + pipeName)
	err := host.ListenAndServe()
	fmt.Println(err)
}
