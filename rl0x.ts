//% color=#444444 icon="\uf012"
//% groups=['On start', 'Variables', 'Optional']
namespace rl0x {

    export enum RADIO {
        //% block="RL01"
        RL01 = 433,
        //% block="RL02"
        RL02 = 868,
        //% block="RL03"
        RL03 = 915
    }

    let initialized = false
    let onReceivedNumberHandler: (receivedNumber: number) => void;
    let onReceivedValueHandler: (name: string, value: number) => void;
    let onReceivedStringHandler: (receivedString: string) => void;

    //%block="RL0x use %u with ID %v"
    //%shim=rl0x::configId
    //%u.defl=0
    export function config(u: RADIO, v: number = 0): void {
        return;
    }

    //%shim=rl0x::send
    function send(u: String): void {
        return;
    }

    //%shim=rl0x::recv
    function recv(): string {
        return "";
    }

    //%block="RL0x send number %u"
    //%u.defl=0
    export function sendNumber(u: number): void {
        send("a" + convertToText(u));
        return;
    }

    //%block="RL0x send string %u"
    //%u.defl="Hello, World!"
    export function sendString(u: string): void {
        send("b" + u)
        return;
    }

    //%block="RL0x send value %str = %val"
    //%str.defl="name"
    //%val.defl="value"
    export function sendValue(str: string, val: number): void {
        send("c" + str + "$!$$!$" + convertToText(val))
        return;
    }

    //%block="RL0x begin"
    //%shim=rl0x::begin
    //%advanced=true
    export function begin(): void {
        return;
    }

    //%block="RL0x set tx power %u"
    //%u.defl=23
    //%shim=rl0x::setTxPower
    //%advanced=true
    export function setTxPower(u: number = 23): void {
        return;
    }
    
    //%shim=rl0x::onDataReceived
    function onDataReceived(body: Action): void {
        return;
    }

    function init() {
        if (initialized) return;
        initialized = true;

        startParallel(function () {

            while(true)
            {

                        let rcvStr = recv()
                        //serial.writeString("Test\n")
                        switch (rcvStr[0]) {
                            case "a":
                                onReceivedNumberHandler(parseFloat(rcvStr.substr(1, rcvStr.length)));
                                break;
                            case "b":
                                onReceivedStringHandler(rcvStr.substr(1, rcvStr.length));
                                break;
                            case "c":
                                let index = rcvStr.indexOf("$!$$!$");
                                onReceivedValueHandler(rcvStr.substr(1, index - 1), parseFloat(rcvStr.substr(index + 6, rcvStr.length)))
                                break;
                            default:
                                return;
                        }
                        rcvStr = ""

            }

        })
    }

    //%block="RL0x last rssi"
    //%shim=rl0x::lastRssi
    //%advanced=true
    export function lastRssi(): number {
        return 1;
    }

    //% block="RL0x on received "
    //% draggableParameters=reporter
    export function onReceivedNumber(cb: (receivedNumber: number) => void): void {
        init();
        onReceivedNumberHandler = cb
    }

    //% block="RL0x on received "
    //% draggableParameters=reporter
    export function onReceivedString(cb: (receivedString: string) => void): void {
            init();
            onReceivedStringHandler = pins.createBuffer(0)
    }

    //% block="RL0x on received "
    //% draggableParameters=reporter
    export function onReceivedValue(cb: (name: string, value: number) => void): void {
        init();
        onReceivedValueHandler = cb
    }

    //% shim=parall::startParallel
    export function startParallel(u: () => void) {
        return 1;
    }

    begin();
    setTxPower(23);
}
