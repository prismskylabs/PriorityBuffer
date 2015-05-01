#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include <boost/filesystem.hpp>

#include "dbfixture.h"
#include "prioritydb.h"

#define DEFAULT_MAX_SIZE 100000000LL


namespace fs = boost::filesystem;

TEST_F(DBFixture, EmptyDBTest) {
    EXPECT_FALSE(fs::exists(db_path_));
}

TEST_F(DBFixture, ConstructDBTest) {
    ASSERT_FALSE(fs::exists(db_path_));
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    EXPECT_TRUE(fs::exists(db_path_));
}

TEST_F(DBFixture, ConstructDBNoDestructTest) {
    ASSERT_FALSE(fs::exists(db_path_));
    {
        PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
        EXPECT_TRUE(fs::exists(db_path_));
    }
    EXPECT_TRUE(fs::exists(db_path_));
}

TEST_F(DBFixture, ConstructDBMultipleTest) {
    ASSERT_FALSE(fs::exists(db_path_));
    {
        PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
        ASSERT_TRUE(fs::exists(db_path_));
    }
    {
        PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
        EXPECT_TRUE(fs::exists(db_path_));
    }
    EXPECT_TRUE(fs::exists(db_path_));
}

TEST_F(DBFixture, ConstructThrowTest) {
    bool thrown = false;
    try {
        PriorityDB db{DEFAULT_MAX_SIZE, (fs::temp_directory_path() / fs::path{""}).native()};
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"unable to open database file"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, ConstructCurrentThrowTest) {
    bool thrown = false;
    try {
        PriorityDB db{DEFAULT_MAX_SIZE, (fs::temp_directory_path() / fs::path{"."}).native()};
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"unable to open database file"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, ConstructParentThrowTest) {
    bool thrown = false;
    try {
        PriorityDB db{DEFAULT_MAX_SIZE, (fs::temp_directory_path() / fs::path{".."}).native()};
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"unable to open database file"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, ConstructZeroSpaceTest) {
    bool thrown = false;
    try {
        PriorityDB db{0LL, db_string_};
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"Must specify a nonzero max_size"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, InitialDBTest) {
    ASSERT_FALSE(fs::exists(db_path_));
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    ASSERT_TRUE(fs::exists(db_path_));
    std::stringstream stream;
    stream << "SELECT name FROM sqlite_master WHERE type='table' AND name='"
           << table_name_
           << "';";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    auto record = response[0];
    ASSERT_EQ(1, record.size());
    ASSERT_NE(record.end(), record.find("name"));
    EXPECT_EQ(std::string{"prism_data"}, record["name"]);
}

TEST_F(DBFixture, InitialEmptyDBTest) {
    ASSERT_FALSE(fs::exists(db_path_));
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    ASSERT_TRUE(fs::exists(db_path_));
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    EXPECT_EQ(0, response.size());
}

TEST_F(DBFixture, InitialDBAfterDestructorTest) {
    ASSERT_FALSE(fs::exists(db_path_));
    {
        PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
        ASSERT_TRUE(fs::exists(db_path_));
    }
    ASSERT_TRUE(fs::exists(db_path_));
    std::stringstream stream;
    stream << "SELECT name FROM sqlite_master WHERE type='table' AND name='"
           << table_name_
           << "';";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    auto record = response[0];
    ASSERT_EQ(1, record.size());
    ASSERT_NE(record.end(), record.find("name"));
    EXPECT_EQ(std::string{"prism_data"}, record["name"]);
}

TEST_F(DBFixture, InitialEmptyDBAfterDestructorTest) {
    ASSERT_FALSE(fs::exists(db_path_));
    {
        PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
        ASSERT_TRUE(fs::exists(db_path_));
    }
    ASSERT_TRUE(fs::exists(db_path_));
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    EXPECT_EQ(0, response.size());
}

TEST_F(DBFixture, InsertEmptyHashTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "", 5, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(0, response.size());
}

TEST_F(DBFixture, InsertSingleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    auto record = response[0];
    ASSERT_EQ(5, record.size());
    EXPECT_EQ(1, std::stoi(record["id"]));
    EXPECT_EQ(1, std::stoi(record["priority"]));
    EXPECT_EQ(std::string{"hash"}, record["hash"]);
    EXPECT_EQ(false, std::stoi(record["on_disk"]));
}

TEST_F(DBFixture, InsertCoupleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    db.Insert(3, "hashbrowns", 10, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    {
        auto record = response[0];
        ASSERT_EQ(5, record.size());
        EXPECT_EQ(1, std::stoi(record["id"]));
        EXPECT_EQ(1, std::stoi(record["priority"]));
        EXPECT_EQ(std::string{"hash"}, record["hash"]);
        EXPECT_EQ(5, std::stoi(record["size"]));
        EXPECT_EQ(false, std::stoi(record["on_disk"]));
    }
    {
        auto record = response[1];
        ASSERT_EQ(5, record.size());
        EXPECT_EQ(2, std::stoi(record["id"]));
        EXPECT_EQ(3, std::stoi(record["priority"]));
        EXPECT_EQ(std::string{"hashbrowns"}, record["hash"]);
        EXPECT_EQ(10, std::stoi(record["size"]));
        EXPECT_EQ(true, std::stoi(record["on_disk"]));
    }
}

TEST_F(DBFixture, InsertManyTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, i % 2);
    }
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(number_of_records, response.size());
    for (int i = 0; i < number_of_records; ++i) {
        auto record = response[i];
        ASSERT_EQ(5, record.size());
        EXPECT_EQ(i + 1, std::stoi(record["id"]));
        EXPECT_EQ(i, std::stoi(record["priority"]));
        EXPECT_EQ(std::to_string(i * i), record["hash"]);
        EXPECT_EQ(i * 2, std::stoi(record["size"]));
        EXPECT_EQ(i % 2, std::stoi(record["on_disk"]));
    }
}

TEST_F(DBFixture, DeleteNullTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(1, response.size());
    }
    db.Delete("");
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
}

TEST_F(DBFixture, DeleteBadHashTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(1, response.size());
    }
    db.Delete("h");
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
}

TEST_F(DBFixture, DeleteSingleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(1, response.size());
    }
    db.Delete("hash");
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(0, response.size());
}

TEST_F(DBFixture, DeleteCoupleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    db.Insert(2, "hashbrowns", 10, true);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(2, response.size());
    }
    db.Delete("hash");
    db.Delete("hashbrowns");
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(0, response.size());
}

TEST_F(DBFixture, DeleteManyTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, i % 2);
    }
    {
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(number_of_records, response.size());
    }
    for (int i = 0; i < number_of_records; ++i) {
        db.Delete(std::to_string(i * i));
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(number_of_records - i - 1, response.size());
    }
}

TEST_F(DBFixture, UpdateNullTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(1, response.size());
    }
    db.Update("", true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    auto record = response[0];
    ASSERT_EQ(5, record.size());
    EXPECT_EQ(1, std::stoi(record["id"]));
    EXPECT_EQ(1, std::stoi(record["priority"]));
    EXPECT_EQ(std::string{"hash"}, record["hash"]);
    EXPECT_EQ(5, std::stoi(record["size"]));
    EXPECT_EQ(false, std::stoi(record["on_disk"]));
}

TEST_F(DBFixture, UpdateBadHashTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(1, response.size());
    }
    db.Update("h", true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    auto record = response[0];
    ASSERT_EQ(5, record.size());
    EXPECT_EQ(1, std::stoi(record["id"]));
    EXPECT_EQ(1, std::stoi(record["priority"]));
    EXPECT_EQ(std::string{"hash"}, record["hash"]);
    EXPECT_EQ(5, std::stoi(record["size"]));
    EXPECT_EQ(false, std::stoi(record["on_disk"]));
}

TEST_F(DBFixture, UpdateSingleFalseToTrueTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(1, response.size());
    }
    db.Update("hash", true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    auto record = response[0];
    ASSERT_EQ(5, record.size());
    EXPECT_EQ(1, std::stoi(record["id"]));
    EXPECT_EQ(1, std::stoi(record["priority"]));
    EXPECT_EQ(std::string{"hash"}, record["hash"]);
    EXPECT_EQ(5, std::stoi(record["size"]));
    EXPECT_EQ(true, std::stoi(record["on_disk"]));
}

TEST_F(DBFixture, UpdateSingleTrueToFalseTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(1, response.size());
    }
    db.Update("hash", false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    auto record = response[0];
    ASSERT_EQ(5, record.size());
    EXPECT_EQ(1, std::stoi(record["id"]));
    EXPECT_EQ(1, std::stoi(record["priority"]));
    EXPECT_EQ(std::string{"hash"}, record["hash"]);
    EXPECT_EQ(5, std::stoi(record["size"]));
    EXPECT_EQ(false, std::stoi(record["on_disk"]));
}

TEST_F(DBFixture, UpdateSingleFalseToFalseTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(1, response.size());
    }
    db.Update("hash", false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    auto record = response[0];
    ASSERT_EQ(5, record.size());
    EXPECT_EQ(1, std::stoi(record["id"]));
    EXPECT_EQ(1, std::stoi(record["priority"]));
    EXPECT_EQ(std::string{"hash"}, record["hash"]);
    EXPECT_EQ(5, std::stoi(record["size"]));
    EXPECT_EQ(false, std::stoi(record["on_disk"]));
}

TEST_F(DBFixture, UpdateSingleTrueToTrueTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(1, response.size());
    }
    db.Update("hash", true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    auto record = response[0];
    ASSERT_EQ(5, record.size());
    EXPECT_EQ(1, std::stoi(record["id"]));
    EXPECT_EQ(1, std::stoi(record["priority"]));
    EXPECT_EQ(std::string{"hash"}, record["hash"]);
    EXPECT_EQ(5, std::stoi(record["size"]));
    EXPECT_EQ(true, std::stoi(record["on_disk"]));
}

TEST_F(DBFixture, UpdateCoupleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    db.Insert(3, "hashbrowns", 10, true);
    { 
        std::stringstream stream;
        stream << "SELECT * FROM "
               << table_name_
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(2, response.size());
    }
    db.Update("hash", true);
    db.Update("hashbrowns", false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    {
        auto record = response[0];
        ASSERT_EQ(5, record.size());
        EXPECT_EQ(1, std::stoi(record["id"]));
        EXPECT_EQ(1, std::stoi(record["priority"]));
        EXPECT_EQ(std::string{"hash"}, record["hash"]);
        EXPECT_EQ(5, std::stoi(record["size"]));
        EXPECT_EQ(true, std::stoi(record["on_disk"]));
    }
    {
        auto record = response[1];
        ASSERT_EQ(5, record.size());
        EXPECT_EQ(2, std::stoi(record["id"]));
        EXPECT_EQ(3, std::stoi(record["priority"]));
        EXPECT_EQ(std::string{"hashbrowns"}, record["hash"]);
        EXPECT_EQ(10, std::stoi(record["size"]));
        EXPECT_EQ(false, std::stoi(record["on_disk"]));
    }
}

TEST_F(DBFixture, UpdateManyTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, i % 2);
        db.Update(std::to_string(i * i), (i + 1) % 2);
    }
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(number_of_records, response.size());
    for (int i = 0; i < number_of_records; ++i) {
        auto record = response[i];
        ASSERT_EQ(5, record.size());
        EXPECT_EQ(i + 1, std::stoi(record["id"]));
        EXPECT_EQ(i, std::stoi(record["priority"]));
        EXPECT_EQ(std::to_string(i * i), record["hash"]);
        EXPECT_EQ(i * 2, std::stoi(record["size"]));
        EXPECT_EQ((i + 1) % 2, std::stoi(record["on_disk"]));
    }
}

TEST_F(DBFixture, HighestHashNoneFalseOnDiskTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(0, response.size());
    bool on_disk = false;
    EXPECT_TRUE(db.GetHighestHash(on_disk).empty());
    EXPECT_FALSE(on_disk);
}

TEST_F(DBFixture, HighestHashNoneTrueOnDiskTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(0, response.size());
    bool on_disk = true;
    EXPECT_TRUE(db.GetHighestHash(on_disk).empty());
    EXPECT_TRUE(on_disk);
}

TEST_F(DBFixture, HighestHashSingleInMemoryTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    {
        bool on_disk = true;
        EXPECT_EQ(std::string{"hash"}, db.GetHighestHash(on_disk));
        EXPECT_FALSE(on_disk);
    }
    {
        bool on_disk = false;
        EXPECT_EQ(std::string{"hash"}, db.GetHighestHash(on_disk));
        EXPECT_FALSE(on_disk);
    }
}

TEST_F(DBFixture, HighestHashSingleOnDiskTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    {
        bool on_disk = true;
        EXPECT_EQ(std::string{"hash"}, db.GetHighestHash(on_disk));
        EXPECT_TRUE(on_disk);
    }
    {
        bool on_disk = false;
        EXPECT_EQ(std::string{"hash"}, db.GetHighestHash(on_disk));
        EXPECT_TRUE(on_disk);
    }
}

TEST_F(DBFixture, HighestHashCoupleInMemoryTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    db.Insert(3, "hashbrowns", 10, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    bool on_disk;
    EXPECT_EQ(std::string{"hashbrowns"}, db.GetHighestHash(on_disk));
    EXPECT_FALSE(on_disk);
}

TEST_F(DBFixture, HighestHashCoupleOnDiskTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    db.Insert(3, "hashbrowns", 10, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    bool on_disk;
    EXPECT_EQ(std::string{"hashbrowns"}, db.GetHighestHash(on_disk));
    EXPECT_TRUE(on_disk);
}

TEST_F(DBFixture, HighestHashCoupleTiedTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    db.Insert(1, "hashbrowns", 10, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    bool on_disk;
    EXPECT_EQ(std::string{"hashbrowns"}, db.GetHighestHash(on_disk));
    EXPECT_FALSE(on_disk);
}

TEST_F(DBFixture, HighestHashCoupleTiedAgainTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    db.Insert(1, "hashbrowns", 10, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    bool on_disk;
    EXPECT_EQ(std::string{"hash"}, db.GetHighestHash(on_disk));
    EXPECT_FALSE(on_disk);
}

TEST_F(DBFixture, HighestHashManyInMemoryTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, (i + 1) % 2);
    }
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(number_of_records, response.size());
    bool on_disk;
    EXPECT_EQ(std::to_string(99 * 99), db.GetHighestHash(on_disk));
    EXPECT_FALSE(on_disk);
}

TEST_F(DBFixture, HighestHashManyOnDiskTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, i % 2);
    }
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(number_of_records, response.size());
    bool on_disk;
    EXPECT_EQ(std::to_string(99 * 99), db.GetHighestHash(on_disk));
    EXPECT_TRUE(on_disk);
}

TEST_F(DBFixture, LowestMemoryHashNoneTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(0, response.size());
    EXPECT_TRUE(db.GetLowestMemoryHash().empty());
}

TEST_F(DBFixture, LowestMemoryHashNoneAgainTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    EXPECT_TRUE(db.GetLowestMemoryHash().empty());
}

TEST_F(DBFixture, LowestMemoryHashSingleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    EXPECT_EQ(std::string{"hash"}, db.GetLowestMemoryHash());
}

TEST_F(DBFixture, LowestMemoryHashCoupleATest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    db.Insert(3, "hashbrowns", 10, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_EQ(std::string{"hash"}, db.GetLowestMemoryHash());
}

TEST_F(DBFixture, LowestMemoryHashCoupleBTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    db.Insert(3, "hashbrowns", 10, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_EQ(std::string{"hashbrowns"}, db.GetLowestMemoryHash());
}

TEST_F(DBFixture, LowestMemoryHashCoupleCTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    db.Insert(3, "hashbrowns", 10, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_EQ(std::string{"hash"}, db.GetLowestMemoryHash());
}

TEST_F(DBFixture, LowestMemoryHashCoupleDTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(3, "hash", 5, false);
    db.Insert(1, "hashbrowns", 10, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_EQ(std::string{"hashbrowns"}, db.GetLowestMemoryHash());
}

TEST_F(DBFixture, LowestMemoryHashCoupleETest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    db.Insert(3, "hashbrowns", 10, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_TRUE(db.GetLowestMemoryHash().empty());
}

TEST_F(DBFixture, LowestMemoryHashManyATest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, i % 2);
    }
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(number_of_records, response.size());
    EXPECT_EQ(std::to_string(0), db.GetLowestMemoryHash());
}

TEST_F(DBFixture, LowestMemoryHashManyBTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, (i + 1) % 2);
    }
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(number_of_records, response.size());
    EXPECT_EQ(std::to_string(1), db.GetLowestMemoryHash());
}

TEST_F(DBFixture, LowestDiskHashNoneTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(0, response.size());
    EXPECT_TRUE(db.GetLowestDiskHash().empty());
}

TEST_F(DBFixture, LowestDiskHashNoneAgainTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    EXPECT_TRUE(db.GetLowestDiskHash().empty());
}

TEST_F(DBFixture, LowestDiskHashSingleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(1, response.size());
    EXPECT_EQ(std::string{"hash"}, db.GetLowestDiskHash());
}

TEST_F(DBFixture, LowestDiskHashCoupleATest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    db.Insert(3, "hashbrowns", 10, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_EQ(std::string{"hash"}, db.GetLowestDiskHash());
}

TEST_F(DBFixture, LowestDiskHashCoupleBTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    db.Insert(3, "hashbrowns", 10, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_EQ(std::string{"hashbrowns"}, db.GetLowestDiskHash());
}

TEST_F(DBFixture, LowestDiskHashCoupleCTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    db.Insert(3, "hashbrowns", 10, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_EQ(std::string{"hash"}, db.GetLowestDiskHash());
}

TEST_F(DBFixture, LowestDiskHashCoupleDTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(3, "hash", 5, true);
    db.Insert(1, "hashbrowns", 10, true);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_EQ(std::string{"hashbrowns"}, db.GetLowestDiskHash());
}

TEST_F(DBFixture, LowestDiskHashCoupleETest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    db.Insert(3, "hashbrowns", 10, false);
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(2, response.size());
    EXPECT_TRUE(db.GetLowestDiskHash().empty());
}

TEST_F(DBFixture, LowestDiskHashManyATest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, i % 2);
    }
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(number_of_records, response.size());
    EXPECT_EQ(std::to_string(1), db.GetLowestDiskHash());
}

TEST_F(DBFixture, LowestDiskHashManyBTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, (i + 1) % 2);
    }
    std::stringstream stream;
    stream << "SELECT * FROM "
           << table_name_
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(number_of_records, response.size());
    EXPECT_EQ(std::to_string(0), db.GetLowestDiskHash());
}

TEST_F(DBFixture, FullEmptyTest) { // Yeah this test name is silly
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    EXPECT_FALSE(db.Full());
}

TEST_F(DBFixture, FullInMemoryUnderTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", DEFAULT_MAX_SIZE - 1, false);
    EXPECT_FALSE(db.Full());
}

TEST_F(DBFixture, FullInMemoryExactTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", DEFAULT_MAX_SIZE, false);
    EXPECT_FALSE(db.Full());
}

TEST_F(DBFixture, FullInMemoryOverTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", DEFAULT_MAX_SIZE + 1, false);
    EXPECT_FALSE(db.Full());
}

TEST_F(DBFixture, FullOnDiskUnderTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", DEFAULT_MAX_SIZE - 1, true);
    EXPECT_FALSE(db.Full());
}

TEST_F(DBFixture, FullOnDiskExactTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", DEFAULT_MAX_SIZE, true);
    EXPECT_FALSE(db.Full());
}

TEST_F(DBFixture, FullOnDiskOverTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", DEFAULT_MAX_SIZE + 1, true);
    EXPECT_TRUE(db.Full());
}

TEST_F(DBFixture, FullMixedOverCoupleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", DEFAULT_MAX_SIZE, true);
    ASSERT_FALSE(db.Full());
    db.Insert(3, "hashbrowns", 1, false);
    EXPECT_FALSE(db.Full());
}

TEST_F(DBFixture, FullOnDiskOverCoupleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", DEFAULT_MAX_SIZE, true);
    ASSERT_FALSE(db.Full());
    db.Insert(3, "hashbrowns", 1, true);
    EXPECT_TRUE(db.Full());
}

TEST_F(DBFixture, FullOnDiskDeleteCoupleTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", DEFAULT_MAX_SIZE, true);
    ASSERT_FALSE(db.Full());
    db.Insert(3, "hashbrowns", 1, true);
    ASSERT_TRUE(db.Full());
    db.Delete("hashbrowns");
    EXPECT_FALSE(db.Full());
}

TEST_F(DBFixture, DeletedDBThrowInsertTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    fs::remove(db_path_);
    ASSERT_FALSE(fs::exists(db_path_));
    bool thrown = false;
    try {
        db.Insert(1, "hash", 5, false);
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"no such table: prism_data"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, DeletedDBThrowDeleteTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    fs::remove(db_path_);
    ASSERT_FALSE(fs::exists(db_path_));
    bool thrown = false;
    try {
        db.Delete("hash");
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"no such table: prism_data"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, DeletedDBThrowUpdateTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    fs::remove(db_path_);
    ASSERT_FALSE(fs::exists(db_path_));
    bool thrown = false;
    try {
        db.Update("hash", true);
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"no such table: prism_data"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, DeletedDBThrowGetHighestHashTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    fs::remove(db_path_);
    ASSERT_FALSE(fs::exists(db_path_));
    bool thrown = false;
    try {
        bool on_disk;
        db.GetHighestHash(on_disk);
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"no such table: prism_data"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, DeletedDBThrowGetLowestMemoryHashTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    fs::remove(db_path_);
    ASSERT_FALSE(fs::exists(db_path_));
    bool thrown = false;
    try {
        db.GetLowestMemoryHash();
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"no such table: prism_data"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, DeletedDBThrowGetLowestDiskashTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    fs::remove(db_path_);
    ASSERT_FALSE(fs::exists(db_path_));
    bool thrown = false;
    try {
        db.GetLowestDiskHash();
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"no such table: prism_data"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, DeletedDBThrowFullTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    fs::remove(db_path_);
    ASSERT_FALSE(fs::exists(db_path_));
    bool thrown = false;
    try {
        db.Full();
    } catch (const PriorityDBException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"no such table: prism_data"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(DBFixture, GetDiskLengthZeroTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    EXPECT_EQ(0, db.GetDiskLength());
}

TEST_F(DBFixture, GetDiskLengthStillZeroTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    EXPECT_EQ(0, db.GetDiskLength());
}

TEST_F(DBFixture, GetDiskLengthOneTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    EXPECT_EQ(1, db.GetDiskLength());
}

TEST_F(DBFixture, GetDiskLengthStillOneTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    db.Insert(3, "hashbrowns", 10, false);
    EXPECT_EQ(1, db.GetDiskLength());
}

TEST_F(DBFixture, GetDiskLengthManyTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, true);
    }
    EXPECT_EQ(number_of_records, db.GetDiskLength());
}

TEST_F(DBFixture, GetDiskLengthManyAlternateTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, i % 2);
    }
    EXPECT_EQ(number_of_records / 2, db.GetDiskLength());
}

TEST_F(DBFixture, GetDiskSizeZeroTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    EXPECT_EQ(0, db.GetDiskSize());
}

TEST_F(DBFixture, GetDiskSizeStillZeroTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, false);
    EXPECT_EQ(0, db.GetDiskSize());
}

TEST_F(DBFixture, GetDiskSizeOneTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    EXPECT_EQ(5, db.GetDiskSize());
}

TEST_F(DBFixture, GetDiskSizeStillOneTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    db.Insert(1, "hash", 5, true);
    db.Insert(3, "hashbrowns", 10, false);
    EXPECT_EQ(5, db.GetDiskSize());
}

TEST_F(DBFixture, GetDiskSizeManyTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, true);
    }
    EXPECT_EQ(99 * number_of_records, db.GetDiskSize());
}

TEST_F(DBFixture, GetDiskSizeManyAlternateTest) {
    PriorityDB db{DEFAULT_MAX_SIZE, db_string_};
    auto number_of_records = 100;
    for (int i = 0; i < number_of_records; ++i) {
        db.Insert(i, std::to_string(i * i), i * 2, i % 2);
    }
    EXPECT_EQ(100 * number_of_records / 2, db.GetDiskSize());
}
