
export enum LinkStatus {
  Up = 1,
  Down = 2
}

export interface NetworkInterface {
  name: string;
  adapterName: string;
  hardwareAddress: number[];
  IP?: string;
  IPv6?: string;
  status: LinkStatus;
  isLoopback: boolean;
}


export interface NetworkHost {
  MAC: number[];
  IPAddress: string;
}


export function listInterfaces(): Promise<NetworkInterface[]>;

export declare class ArpMonitor {
  start(intf: NetworkInterface, callback: (host: NetworkHost) => void): Promise<void>;
  stop(): Promise<void>;
}
