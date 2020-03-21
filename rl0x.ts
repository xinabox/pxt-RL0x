//% color=#444444 icon="\uf012"
//% groups=['On start', 'Variables', 'Optional']
namespace RL0X {

    export enum RADIO {
        //% block="RL01"
        RL01 = 433,
        //% block="RL02"
        RL02 = 868,
        //% block="RL03"
        RL03 = 915
    }

    //%block="RL0x use %u with ID %v"
    //%shim=rl0x::configId
    //%u.defl=RADIO.RL03
    //%v.defl=0
    export function config(u: RADIO, v: number = 0): void {
        return;
    }

    //%block="RL0x send number %u"
    //%shim=rl0x::sendNumber
    export function sendNumber(u: number): void {
        return;
    }

    //%block="RL0x send string %u"
    //%u.defl="Hello, World!"
    //%shim=rl0x::sendString
    export function sendString(u: string): void {
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

    //%block="RL0x receive"
    //%shim=rl0x::recv
    export function recv(): string {
        return "xinabox";
    }

    //%block="RL0x last rssi"
    //%shim=rl0x::lastRssi
    //%advanced=true
    export function lastRssi(): number {
        return 1;
    }

    begin();
    setTxPower(23);
}