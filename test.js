const { ArpMonitor, listInterfaces } = require('.')

const arpMonitors = []

listInterfaces().then((list) => {
  const interfaces = list.filter((intf) => !intf.isLoopback && intf.status == 1)

  if (interfaces.length > 0) {
    console.log(`Found ${interfaces.length} network interfaces with link status "up"`)
    console.log('---------------------------------------------------------------------')
    console.log(interfaces)
    console.log('---------------------------------------------------------------------')
    
    console.log('Starting ARP monitors ...')
  
    interfaces.forEach((intf) => {
      const monitor = new ArpMonitor()
      monitor.start(intf, (host) => {
        console.log('Host found:', host.IPAddress)
      })

      arpMonitors.push(monitor)
    })
  } else {
    console.log(`No network interfaces found with link status "up"`)
  }  
})
