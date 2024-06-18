#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void firesbaseInit(String API_KEY, String USER_EMAIL, String USER_PASSWORD){

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(false);
    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
    // Limit the size of response payload to be collected in FirebaseData
    fbdo.setResponseSize(2048);
    Firebase.begin(&config, &auth);
    // You can use TCP KeepAlive in FirebaseData object and tracking the server connection status, please read this for detail.
    // https://github.com/mobizt/Firebase-ESP-Client#about-firebasedata-object
    // fbdo.keepAlive(5, 5, 1);
}

bool firebaseSend(String user_uid, String FIREBASE_PROJECT_ID, unsigned long count, float data){// count and mills
// Firebase.ready() should be called repeatedly to handle authentication tasks.

    Serial.print("Commit a document (append map value in document)... ");

    // The dyamic array of write object firebase_firestore_document_write_t.
    std::vector<struct firebase_firestore_document_write_t> writes;

    // A write object that will be written to the document.
    struct firebase_firestore_document_write_t update_write;

    // Set the write object write operation type.
    // firebase_firestore_document_write_type_update,
    // firebase_firestore_document_write_type_delete,
    // firebase_firestore_document_write_type_transform
    update_write.type = firebase_firestore_document_write_type_update;

    // Set the document content to write (transform)

    FirebaseJson content;
    String documentPath = user_uid;
    documentPath.concat("/b");
    documentPath.concat(String(count));

    content.set("fields/myInteger/doubleValue" , String(data));
    // Set the update document content
    update_write.update_document_content = content.raw();

    //update_write.update_masks = "myMap.key" + String(count);

    // Set the update document path
    update_write.update_document_path = documentPath.c_str();

    // Add a write object to a write array.
    writes.push_back(update_write);

    // A write object that will be written to the document.
    struct firebase_firestore_document_write_t transform_write;
    transform_write.type = firebase_firestore_document_write_type_transform;

    // Set the document path of document to write (transform)
    transform_write.document_transform.transform_document_path = documentPath;

    // Set a transformation of a field of the document.
    struct firebase_firestore_document_write_field_transforms_t field_transforms;

    // Set field path to write.
    field_transforms.fieldPath = "timestamp";
    //field_transforms.fieldPath = "myMap.key" + String(count) + ".timestamp";

    field_transforms.transform_type = firebase_firestore_transform_type_set_to_server_value;

    // Set the transformation content, server value for this case.
    // See https://firebase.google.com/docs/firestore/reference/rest/v1/Write#servervalue
    field_transforms.transform_content = "REQUEST_TIME"; // set timestamp to timestamp field

    // Add a field transformation object to a write object.
    transform_write.document_transform.field_transforms.push_back(field_transforms);

    // Add a write object to a write array.
    writes.push_back(transform_write);

    if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of firebase_firestore_document_write_t */, "" /* transaction */)){
        Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        return true;
    }else{
        Serial.println(fbdo.errorReason());
        return false;
    }
}

bool firebaseReady(){
    if (Firebase.ready()){
        return true;
    }else{
        return false;
    }
}


