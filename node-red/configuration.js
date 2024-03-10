module.exports = function(RED) {
    var crc = require('crc/crc16ccitt');
    var cobs = require('cobs');

    function Configuration(config) {
        RED.nodes.createNode(this,config);
        var node = this;

        node.buldConfiguration = function(data){

            let output = Buffer.alloc(4);
            output[0] = 0xC9 //cbs_configurationStateCommand 
            output[1] = data.baudrate;
            output[2] = data.fdBaudrate;
            let bus = 0;
            if(data.enable) bus |= 0x01;
            if(data.automaticRetransmission) bus |= 0x02;
            if(data.silentMode) bus |= 0x04;
            output[3] = bus;

            return output;
        }

        node.on('input', function(msg) {

            var configData = node.buldConfiguration(msg.payload)
 
            let crcVal = crc(configData);
            let crcBuffer = Buffer.alloc(2);
            crcBuffer[0] = (crcVal>>8)&0xFF
            crcBuffer[1] = crcVal&0xFF
            let output = Buffer.concat([configData,crcBuffer]);
            
            output = cobs.encode(output);
            output = Buffer.concat([Buffer.from([0]),output,Buffer.from([0])]);
            msg.payload = output;
            
            node.send(msg);
        });
    }
    RED.nodes.registerType("Configuration", Configuration);
}
