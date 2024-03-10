module.exports = function(RED) {
    var crc = require('crc/crc16ccitt');
    var cobs = require('cobs');

    function Decode(config) {
        RED.nodes.createNode(this,config);
        var node = this;

        node.docodeMessage = function(data){
            let output;
            const frame = data.slice(0, data.length-1); // remove trailing null
            if(crc(frame) != 0){
                node.warn("CRC error")
                return;
            }
            const payload = data.slice(1,data.length-3);
            switch(data[0]){
                case 0x00: node.docodeDataFrame(payload);
                case 0x09: node.docodeConfigurationState(payload);
            }   
        }

        node.docodeDataFrame = function(data){
            let canId = 0;
            canId |= (data[0] << 24) & 0xFF000000;
            canId |= (data[1] << 16) & 0x00FF0000;
            canId |= (data[2] << 8) & 0x0000FF00;
            canId |= (data[3]) & 0x000000FF;
            const flags = data[4];
            let ext = false;
            if(flags&0x01) ext = true;
            let fd = false;
            if(flags&0x02) fd = true;
            let rtr = false;
            if(flags&0x04) rtr = true;
            const dlc = (flags>>4)&0x0F;

            let outMsg={
                payload:{
                    canid: canId,
                    ext:ext,
                    fd:fd,
                    rtr:rtr,
                    dlc:dlc,
                    data: data.slice(5)
                }
            }
            node.send([outMsg,undefined])
        }

        node.docodeConfigurationState = function(data){
            const baudrate = data[0];
            const fdBaudrate = data[1];
            const flags = data[2];

            let enable = false;
            if(flags&0x01) enable = true;
            let automaticRetransmission = false;
            if(flags&0x02) automaticRetransmission = true;
            let silentMode = false;
            if(flags&0x04) silentMode = true;

            let outMsg={
                payload:{
                    baudrate: baudrate,
                    fdBaudrate: fdBaudrate,
                    enable:enable,
                    automaticRetransmission:automaticRetransmission,
                    silentMode:silentMode
                }
            }
            
            node.send([undefined,outMsg])
        }

        node.rxData = new Uint8Array([]);
        node.rxBuffer = function(data){
            if(!Buffer.isBuffer(data)){
                node.warn("The received data is not a buffer. Message ignored!")
                return [];
            }
            node.rxData = new Uint8Array([...node.rxData, ...new Uint8Array(data)]);
            return node.handleBuffer();
        }

        node.handleBuffer = function(){
            let output = [];
            let nullIndex = node.rxData.indexOf(0); 
            while(nullIndex !== -1){
                const returnData =  node.rxData.subarray(0, nullIndex+1)
                node.rxData = node.rxData.subarray(nullIndex+1)
                if(returnData.size === 0) continue;
                
                output.push(Buffer.from(returnData));
                nullIndex = node.rxData.indexOf(0);
            }
            return output;
        }

        node.on('input', function(msg) {
            let frames = node.rxBuffer(msg.payload);
            frames.forEach(element => {
                const message = cobs.decode(element);
                if(message.length !== 0) {
                    node.docodeMessage(message)
                }
            });
        });
    }
    RED.nodes.registerType("Decoder", Decode);
}