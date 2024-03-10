module.exports = function(RED) {
    var crc = require('crc/crc16ccitt');
    var cobs = require('cobs');

    function Encode(config) {
        RED.nodes.createNode(this,config);
        var node = this;

        node.lengthToDlc = function(length){
            const dlc =  [0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64];

            if(length > 64) return -1;
            if(length <= 8) return length;

            for(let i = 8; i < dlc.length; i++)
            {
                if(dlc[i] >= length) return i;
            }

            return -1;
        }

        node.buldMessage = function(data){

            let canHeader = new Buffer.alloc(6);
            const canData = new Buffer.from(data.data);

            let flags = 0;
            if(data.ext) flags |= 0x01;
            if(data.fd) flags |= 0x02;
            if(data.rtr) flags |= 0x04;

            const dlc = node.lengthToDlc(canData.length);
            if(dlc === -1){
                node.warn("DLC Error");
                return;
            }
            flags |= (dlc<<4)&0xF0;

            canHeader[0] = 0; // data massage payload Id
            canHeader[1] = (data.canid >> 24) & 0xFF;
            canHeader[2] = (data.canid >> 16) & 0xFF;
            canHeader[3] = (data.canid >> 8) & 0xFF;
            canHeader[4] = (data.canid) & 0xFF;
            canHeader[5] = flags & 0xFF;

            return Buffer.concat([canHeader, canData]);
        }

        node.on('input', function(msg) {

            var configData = node.buldMessage(msg.payload)
            if(configData === null) return;
 
            let crcVal = crc(configData);
            let crcBuffer = Buffer.alloc(2);
            crcBuffer[0] = (crcVal>>8)&0xFF
            crcBuffer[1] = crcVal&0xFF

            let output = Buffer.concat([configData,crcBuffer]);
            output = cobs.encode(output);
            output = Buffer.concat([Buffer.from([0]), output, Buffer.from([0])]);
            msg.payload = output;
            
            node.send(msg);
        });
    }
    RED.nodes.registerType("Encoder", Encode);
}
